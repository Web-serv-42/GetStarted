/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:23:42 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/14 16:11:04 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

#include <cstring>
#include <cstdlib>

#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

class CGI
{
	private:
		std::string					m_Interpreter;
		std::string					m_ScriptPath;
		std::vector<std::string>	m_EnvVars;
		std::string					m_RequestBody;
		std::pair<int, std::string>	m_TmpFileBody;

		char**						m_Envp;
		char**						m_Argv;
		// Temp var
		std::vector<char*>			m_EnvpStrings;
		std::vector<char*>			m_ArgvStrings;

		pid_t						m_Pid;
		int							m_PipeInFd[2];
		int							m_PipeOutFd[2];

		size_t						m_BodyBytesSent;
		std::string					m_OutputBuffer;
	public:
		CGI();
		CGI(std::string interpreter, std::string scriptPath, std::vector<std::string> envVars, std::string body);
		~CGI();

		bool	Run();
		void	ClearInheritedFds(int pipeIn, int pipeOut);
		void	InitEnvpAndArgv();

		bool	SendBodyToScript();
		bool	ReadOutputFromScript();

		int		GetPipeInFd();
		int		GetPipeOutFd();
		void	ClosePipeIn();
		void	ClosePipeOut();

};
