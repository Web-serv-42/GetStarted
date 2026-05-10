/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:24:00 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/11 00:46:09 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGI.hpp"
#include "Core/Timer.hpp"
#include "Core/Log.hpp"

#define BUFFER_SIZE 4096

CGI::CGI()
{
}

CGI::CGI(std::string interpreter, std::string scriptPath, std::vector<std::string> envVars, std::string body)
	: m_Interpreter(interpreter), m_ScriptPath(scriptPath), m_EnvVars(envVars),
	m_RequestBody(body)
{
	this->m_Pid = -1;
	this->m_Pid = -1;
    this->m_PipeInFd[0] = -1;
	this->m_PipeInFd[1] = -1;
    this->m_PipeOutFd[0] = -1;
	this->m_PipeOutFd[1] = -1;
	this->m_Envp = NULL;
	this->m_Argv = NULL;
	this->m_BodyBytesSent = 0;
}

CGI::~CGI()
{
	if (this->m_Pid > 0) // Add this safety check
    {
		kill(this->m_Pid, SIGKILL);
		waitpid(this->m_Pid, NULL, WNOHANG);
	}
	this->ClosePipeIn();
    this->ClosePipeOut();
}

bool	CGI::Run()
{
	// Build this->m_Envp and this->m_Argv
	this->InitEnvpAndArgv();

	// Setup pipes
	if ((pipe(this->m_PipeInFd) == -1) || (pipe(this->m_PipeOutFd) == -1))
	{
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
		// Close the ends of the pipes the child doesn't need
		close(this->m_PipeInFd[1]);
		close(this->m_PipeOutFd[0]);

		// Redirect from stdin to pipeIn [RequestBody]
		if (dup2(this->m_PipeInFd[0], STDIN_FILENO) == -1)
		{
			ERROR_LOG("dup2 failed!");
			std::exit(EXIT_FAILURE);
		}
		close(this->m_PipeInFd[0]);
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
		fcntl(this->m_PipeInFd[1], F_SETFL, O_NONBLOCK);
		fcntl(this->m_PipeOutFd[0], F_SETFL, O_NONBLOCK);
        
        // Close the ends of the pipes the parent doesn't need
		close(this->m_PipeInFd[0]);
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

bool	CGI::SendBodyToScript()
{
	// write()
	int	bytesSent = 0;

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
		return (false);
	}
	if (this->m_BodyBytesSent >= this->m_RequestBody.length())
	{
		// Finished sending
		return (true);
	}
	return (false); // Still more to send
}

bool	CGI::ReadOutputFromScript()
{
	// read() + waitpid
	char	buffer[BUFFER_SIZE];
	int		bytesRead = 0;

	memset((void*)&buffer, 0, BUFFER_SIZE);
	bytesRead = read(this->m_PipeOutFd[0], buffer, BUFFER_SIZE);
	if (bytesRead > 0)
	{
		// Keep reading
		std::string outputedStr(buffer, bytesRead);
		this->m_OutputBuffer.append(buffer, bytesRead);
		SUCCESS_LOG("CGI Outputed: " + outputedStr);
		return (false);
	}
	else if (bytesRead == 0)
	{
		// Finished reading (EOF)
		waitpid(this->m_Pid, NULL, WNOHANG);
		return (true);
	}
	else
	{
		// Error [Bug ? Timeout]
		ERROR_LOG("Error while reading from CGI output");
		return (false);
	}
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
