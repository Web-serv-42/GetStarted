/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:24:00 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/07 16:24:54 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGI.hpp"
#include "Core/Timer.hpp"

CGI::CGI()
{
}

CGI::CGI(std::string interpreter, std::string scriptPath, std::vector<std::string> envVars, std::string body)
	: m_Interpreter(interpreter), m_ScriptPath(scriptPath), m_EnvVars(envVars), m_RequestBody(body)
{
	this->m_Envp = NULL;
	this->m_Argv = NULL;
}

CGI::~CGI()
{
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

	// Child
	if (this->m_Pid == 0)
	{
		// Redirect from stdin to pipeIn [RequestBody]
		// Redirect from stdout to pipeOut [CgiResponse]
		if (dup2(this->GetPipeInFd(), STDIN_FILENO) == -1
			|| dup2(this->GetPipeInFd(), STDOUT_FILENO) == -1)
		{
			return (false);
		}

		this->ClosePipes();
		// Make pipes non blobking [Add them to epoll]
		fcntl(STDIN_FILENO, F_SETFD, O_NONBLOCK);
		fcntl(STDOUT_FILENO, F_SETFD, O_NONBLOCK);

		// Exevce with correct parametres
		if (execve(this->m_Interpreter.c_str(), this->m_Argv, this->m_Envp) == -1)
		{
			return (false);
		}
	}

	// Parent
	this->ClosePipes();
	// Stop after getting response or timeout
	return (true);
}

void	CGI::InitEnvpAndArgv()
{
	std::vector<char*>	cstrings;
	cstrings.reserve(this->m_EnvVars.size());

	for (size_t i = 0; i < this->m_EnvVars.size(); i++)
	{
		cstrings.push_back(const_cast<char*>(this->m_EnvVars[i].c_str()));
	}
	this->m_Envp = &cstrings[0];
}

bool	CGI::SendBodyToScript()
{
	// write()
	return (true);
}

bool	CGI::ReadOutputFromScript()
{
	// read() + waitpid
	return (true);
}

void	CGI::ClosePipes()
{
	close(this->m_PipeInFd[0]);
	close(this->m_PipeInFd[1]);
	close(this->m_PipeOutFd[0]);
	close(this->m_PipeOutFd[1]);
}

int		CGI::GetPipeInFd()
{
	return (this->m_PipeInFd[0]);
}

int		CGI::GetPipeOutFd()
{
	return (this->m_PipeOutFd[1]);
}
