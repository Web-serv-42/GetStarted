/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:24:00 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/18 00:18:57 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGI.hpp"
#include "Core/Timer.hpp"
#include "Core/Log.hpp"

#define BUFFER_SIZE 4096

CGI::CGI()
{
}

CGI::CGI(std::string interpreter, std::string scriptPath, std::vector<std::string> envVars, std::string bodyOrFile, BodyStorage mode)
	: m_Interpreter(interpreter), m_ScriptPath(scriptPath), m_EnvVars(envVars), m_BodyStorage(mode)
{
	this->m_Pid = -1;
	this->m_PipeInFd[0] = -1;
	this->m_PipeInFd[1] = -1;
	this->m_PipeOutFd[0] = -1;
	this->m_PipeOutFd[1] = -1;
	this->m_Envp = NULL;
	this->m_Argv = NULL;
	this->m_BodyBytesSent = 0;
	if (this->m_BodyStorage == BODY_IN_STRING)
		this->m_RequestBody = bodyOrFile;
	else if (this->m_BodyStorage == BODY_IN_FILE)
		this->m_TmpBodyFile = bodyOrFile;
}

CGI&	CGI::operator=(const CGI& copy)
{
	// Later
	(void)copy;
	return (*this);
}

CGI::~CGI()
{
	if (this->m_Pid > 0) // Add this safety check
	{
		kill(this->m_Pid, SIGKILL);
		waitpid(this->m_Pid, NULL, WNOHANG);
	}
	//TODO: uncomment this after request part is done
	// if (this->m_BodyStorage == BODY_IN_FILE)
	// {
	// 	unlink(this->m_TmpBodyFile.c_str());
	// }
	// Tmp_File FD ?
	this->ClosePipeIn();
	this->ClosePipeOut();
}

bool	CGI::Run()
{
	// Build this->m_Envp and this->m_Argv
	this->InitEnvpAndArgv();

	// Setup pipes
	this->SetupPipes();

	int	inputFd = this->m_BodyStorage == BODY_IN_STRING ? 
					this->m_PipeInFd[0] 
					: open(this->m_TmpBodyFile.c_str(), O_RDONLY);
	if (inputFd == -1)
	{
		// Close Pipes
		return (false);
	}
	// Fork new process [Start timer ?]
	this->m_Pid = fork();
	if (this->m_Pid == -1)
	{
		// Close pipes
		return (false);
	}
	// --- CHILD PROCESS (The Script) ---
	if (this->m_Pid == 0)
	{
		this->ClearInheritedFds(inputFd, this->m_PipeOutFd[1]);
		// Close the ends of the pipes the child doesn't need
		if (this->m_BodyStorage == BODY_IN_STRING)
			close(this->m_PipeInFd[1]);
		close(this->m_PipeOutFd[0]);

		// Redirect from stdin to pipeIn [RequestBody]
		if (dup2(inputFd, STDIN_FILENO) == -1)
		{
			ERROR_LOG("dup2 failed!");
			std::exit(EXIT_FAILURE);
		}
		close(inputFd);
		// Redirect from stdout to pipeOut [CgiResponse]
		if (dup2(this->m_PipeOutFd[1], STDOUT_FILENO) == -1)
		{
			ERROR_LOG("dup2 failed!");
			std::exit(EXIT_FAILURE);
		}
		close(this->m_PipeOutFd[1]);

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
		if (this->m_BodyStorage == BODY_IN_STRING)
			fcntl(this->m_PipeInFd[1], F_SETFL, O_NONBLOCK);
		fcntl(this->m_PipeOutFd[0], F_SETFL, O_NONBLOCK);
		
		// Close the ends of the pipes the parent doesn't need
		close(inputFd);
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
	this->m_ArgvStrings.push_back(const_cast<char*>(this->m_ScriptPath.c_str()));
	this->m_ArgvStrings.push_back(NULL); // Null-terminate for execve
	
	this->m_Argv = &this->m_ArgvStrings[0];
}

// ======================= write() && read() =======================
bool	CGI::SendBodyToScript()
{
	// write()
	int	bytesSent = 0;

	if (this->m_BodyBytesSent >= this->m_RequestBody.length())
        return (true);
	//TODO Member 1: Max Body Size check
	bytesSent = write(this->m_PipeInFd[1],
						this->m_RequestBody.c_str() + this->m_BodyBytesSent,
						this->m_RequestBody.length() - this->m_BodyBytesSent);
	if (bytesSent > 0)
	{
		this->m_BodyBytesSent += bytesSent;
	}
	else if (bytesSent == -1)
	{
		// Error [Bug ? Timeout]
		ERROR_LOG("Error while writing to CGI input");
		return (true);
	}
	// If true: Finished, otherwise: still more to send
	return (this->m_BodyBytesSent >= this->m_RequestBody.length());
}

bool	CGI::ReadOutputFromScript()
{
	// read() + waitpid
	char	buffer[BUFFER_SIZE];
	int		bytesRead = 0;
	int		status;

	bytesRead = read(this->m_PipeOutFd[0], buffer, BUFFER_SIZE);
	if (bytesRead > 0)
	{
		// Keep reading
		//TODO" Memeber 1/2: open a new tmp file append bytes 
		std::string outputedStr(buffer, bytesRead);
		this->m_OutputBuffer.append(buffer, bytesRead);
		SUCCESS_LOG("CGI Outputed: " + outputedStr);
		return (false);
	}
	else if (bytesRead == 0)
	{
		// Finished reading (EOF)
		waitpid(this->m_Pid, &status, WNOHANG);
		return (true);
	}
	else
	{
		// Error [Bug ? Timeout]
		ERROR_LOG("Error while reading from CGI output");
		return (true);
	}
}


// ======================= Pipes && Fds =======================
void	CGI::ClearInheritedFds(int pipeIn, int pipeOut)
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
		if (fd > 2 && fd != pipeIn && fd != pipeOut)
		{
			DEBUG_LOG(entry->d_name);
			fdsToClose.push_back(fd);
		}
	}
	closedir(dir);
	for (size_t i = 0; i < fdsToClose.size(); i++)
	{
		close(fdsToClose[i]);
	}
}

bool	CGI::SetupPipes()
{
	if (this->m_BodyStorage == BODY_IN_STRING)
	{
		if ((pipe(this->m_PipeInFd) == -1) || (pipe(this->m_PipeOutFd) == -1))
		{
			return (false);
		}
	}
	else if (this->m_BodyStorage == BODY_IN_FILE)
	{
		if ((pipe(this->m_PipeOutFd) == -1))
		{
			return (false);
		}
	}
	return (true);
}

int		CGI::GetPipeInFd()
{
	return (this->m_PipeInFd[1]); // Write end
}

int		CGI::GetPipeOutFd()
{
	return (this->m_PipeOutFd[0]); // Read end
}

// Add these safe closer methods
void	CGI::ClosePipeIn()
{
	if (this->m_PipeInFd[1] != -1)
	{
		close(this->m_PipeInFd[1]);
		this->m_PipeInFd[1] = -1; // Prevent double-close
	}
}

void	CGI::ClosePipeOut()
{
	if (this->m_PipeOutFd[0] != -1)
	{
		close(this->m_PipeOutFd[0]);
		this->m_PipeOutFd[0] = -1; // Prevent double-close
	}
}

BodyStorage	CGI::GetBodyStorage() const
{
	return (this->m_BodyStorage);
}

