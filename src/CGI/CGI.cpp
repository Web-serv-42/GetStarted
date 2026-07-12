/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:24:00 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/23 11:48:12 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGI.hpp"
#include "Core/Timer.hpp"
#include "Core/Log.hpp"

#define BUFFER_SIZE 4096

CGI::CGI()
{
}
CGI::CGI(std::string interpreter, std::string scriptPath, std::string scriptName, std::vector<std::string> envVars, bool hasBody, std::string tmpBodyFile, std::string tmpOutputFile)
	: m_Interpreter(interpreter), m_ScriptPath(scriptPath), m_ScriptName(scriptName), m_EnvVars(envVars), m_HasBody(hasBody), m_TmpBodyFile(tmpBodyFile),  m_TmpOutputFile(tmpOutputFile)
{
	this->m_Pid = -1;
	this->m_TmpBodyFileFd = -1;
	this->m_TmpOutputFileFd = -1;
	this->m_PipeOutFd[0] = -1;
	this->m_PipeOutFd[1] = -1;
	this->m_Envp = NULL;
	this->m_Argv = NULL;
	this->m_BodyBytesSent = 0;
}
CGI&	CGI::operator=(const CGI& copy)
{
	// Later
	(void)copy;
	return (*this);
}

CGI::~CGI()
{
	// Check if process is still alive before touching it
	if (this->m_Pid != -1)
	{
		kill(this->m_Pid, SIGKILL);
		waitpid(this->m_Pid, NULL, 0); // Block until it's dead to prevent zombies!
	}
	this->ClosePipeOut();
	// Body File
	if (this->m_TmpBodyFileFd != -1)
	{
		close(this->m_TmpBodyFileFd);
	}
	if (this->m_TmpOutputFileFd != -1)
	{
		close(this->m_TmpOutputFileFd);
	}
	// Output File
	if (!this->m_TmpBodyFile.empty())
	{
		std::remove(this->m_TmpBodyFile.c_str());
	}
	if (!this->m_TmpOutputFile.empty())
	{
		std::remove(this->m_TmpOutputFile.c_str());
	}
}

bool	CGI::Run()
{
	this->m_Timer.Reset();
	// Build this->m_Envp and this->m_Argv
	this->InitEnvpAndArgv();

	// Open the output file ONCE right here
    this->m_TmpOutputFileFd = open(this->m_TmpOutputFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (this->m_TmpOutputFileFd == -1)
    {
        return (false);
    }

	// Setup pipes
	if ((pipe(this->m_PipeOutFd) == -1))
	{
		return (false);
	}

	// Fork new process [Start timer ?]
	this->m_Pid = fork();
	if (this->m_Pid == -1)
	{
		close(this->m_PipeOutFd[0]);
		close(this->m_PipeOutFd[1]);
		if (this->m_TmpOutputFileFd != -1)
		{
			close(this->m_TmpOutputFileFd);
			this->m_TmpOutputFileFd = -1;
		}
		return (false);
	}
	// --- CHILD PROCESS (The Script) ---
	if (this->m_Pid == 0)
	{
		// Close the ends of the pipes the child doesn't need
		close(this->m_PipeOutFd[0]);
		this->ClearInheritedFds(this->m_PipeOutFd[1]);
		this->RedirectIO();
		// run in the correct directory
		if (chdir(this->m_ScriptPath.c_str()) != 0)
		{
			ERROR_LOG("chdir failed!");
			std::exit(EXIT_FAILURE);
		}
		// Exevce with correct parametres
		if (execve(this->m_Interpreter.c_str(), this->m_Argv, this->m_Envp) == -1)
		{
			ERROR_LOG("execve failed!");
			std::exit(EXIT_FAILURE);
		}
	}
	else
	{
		// --- PARENT PROCESS (Webserv Engine) ---

		// Make pipes non blobking [Add them to epoll]
		fcntl(this->m_PipeOutFd[0], F_SETFL, O_NONBLOCK);
		
		// Close the ends of the pipes the parent doesn't need
		close(this->m_PipeOutFd[1]);
		// Stop after getting response or timeout
		return (true);
	}
	return (true);
}

void	CGI::InitEnvpAndArgv()
{
	// Initialize Envp
	this->m_EnvpStrings.clear();
	this->m_EnvpStrings.reserve(this->m_EnvVars.size());

	for (size_t i = 0; i < this->m_EnvVars.size(); i++)
	{
		this->m_EnvpStrings.push_back(const_cast<char*>(this->m_EnvVars[i].c_str()));
	}
	this->m_EnvpStrings.push_back(NULL); // Null-terminate for execve
	this->m_Envp = &this->m_EnvpStrings[0];

	// Initialize Argv
	this->m_ArgvStrings.clear();
	this->m_ArgvStrings.push_back(const_cast<char*>(this->m_Interpreter.c_str()));
	this->m_ArgvStrings.push_back(const_cast<char*>(this->m_ScriptName.c_str()));
	this->m_ArgvStrings.push_back(NULL); // Null-terminate for execve
	
	this->m_Argv = &this->m_ArgvStrings[0];
}

// ======================= write() && read() =======================
bool	CGI::ReadOutputFromScript()
{
	// read() + waitpid
	char	buffer[BUFFER_SIZE];
	int		bytesRead = 0;
	int		status = 0;

	bytesRead = read(this->m_PipeOutFd[0], buffer, BUFFER_SIZE);
	if (bytesRead > 0)
	{
		if (this->m_TmpOutputFileFd != -1)
        {
            write(this->m_TmpOutputFileFd, buffer, bytesRead);
        }
		return (false); // Not done yet, more data coming from CGI script
	}
	else if (bytesRead == 0)
	{
		DEBUG_LOG("CGI Output ready");
		// 💡 Fix: Close the output file descriptor here!
		if (this->m_TmpOutputFileFd != -1)
		{
			close(this->m_TmpOutputFileFd);
			this->m_TmpOutputFileFd = -1;
		}
		// Finished reading (EOF)
		waitpid(this->m_Pid, &status, WNOHANG);
		this->m_Pid = -1;
		return (true);
	}
	else
	{
		ERROR_LOG("Error while reading from CGI output");
		return (true);
	}
}

// ======================= Pipes && Fds =======================
void	CGI::ClearInheritedFds(int pipeOut)
{
	std::vector<int>	fdsToClose;
	DIR*				dir = opendir("/proc/self/fd");
	struct  dirent*		entry;
	int					fd;	
	
	if (!dir)
		return;
	while ((entry = readdir(dir)) != NULL)
	{
		fd = std::atoi(entry->d_name);
		if (fd > 2 && fd != pipeOut)
		{
			fdsToClose.push_back(fd);
		}
	}
	closedir(dir);
	for (size_t i = 0; i < fdsToClose.size(); i++)
	{
		close(fdsToClose[i]);
	}
}

void	CGI::RedirectIO()
{
	if (this->m_HasBody)
	{
		this->m_TmpBodyFileFd = open(this->m_TmpBodyFile.c_str(), O_RDONLY);
		if (this->m_TmpBodyFileFd == -1)
		{
			ERROR_LOG("open failed!");
			std::exit(EXIT_FAILURE);
		}
		// Redirect from stdin to inFd [RequestBody]
		if (dup2(this->m_TmpBodyFileFd, STDIN_FILENO) == -1)
		{
			ERROR_LOG("dup2 failed!");
			std::exit(EXIT_FAILURE);
		}
		close(this->m_TmpBodyFileFd);
	}
	else
	{
		int devNull = open("/dev/null", O_RDONLY);
		if (devNull == -1)
		{
			ERROR_LOG("open failed!");
			std::exit(EXIT_FAILURE);
		}
		// Redirect from stdin to /dev/null [RequestBody]
		if (dup2(devNull, STDIN_FILENO) == -1)
		{
			ERROR_LOG("dup2 failed!");
			std::exit(EXIT_FAILURE);
		}
		close(devNull);
	}
	
	// Redirect from stdout to pipeOut [CgiResponse]
	if (dup2(this->m_PipeOutFd[1], STDOUT_FILENO) == -1)
	{
		ERROR_LOG("dup2 failed!");
		std::exit(EXIT_FAILURE);
	}
	close(this->m_PipeOutFd[1]);
}

std::string		CGI::GetTmpOutputFile() const
{
	return (this->m_TmpOutputFile);
}

TimerBenchmark	CGI::GetTimer() const
{
	return (this->m_Timer);
}

int		CGI::GetPipeOutFd()
{
	return (this->m_PipeOutFd[0]); // Read end
}

void	CGI::ClosePipeOut()
{
	if (this->m_PipeOutFd[0] != -1)
	{
		close(this->m_PipeOutFd[0]);
		this->m_PipeOutFd[0] = -1; // Prevent double-close
	}
}
