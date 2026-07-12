/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:23:42 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/23 12:14:13 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Timer.hpp"

#include <string>
#include <vector>

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

class CGI
{
	private:
		std::string					m_Interpreter;
		std::string					m_ScriptPath;
		std::string					m_ScriptName;
		std::vector<std::string>	m_EnvVars;

		bool						m_HasBody;
		std::string					m_TmpBodyFile;
		std::string					m_TmpOutputFile;

		char**						m_Envp;
		char**						m_Argv;
		// Temp var
		std::vector<char*>			m_EnvpStrings;
		std::vector<char*>			m_ArgvStrings;

		pid_t						m_Pid;
		int							m_TmpBodyFileFd;
		int							m_TmpOutputFileFd;
		int							m_PipeOutFd[2];

		size_t						m_BodyBytesSent;
		std::string					m_OutputBuffer;

		TimerBenchmark				m_Timer;
	public:
		CGI();
		CGI(std::string interpreter, std::string scriptPath, std::string scriptName, std::vector<std::string> envVars, bool hasBody, std::string tmpBodyFile, std::string tmpOutputFile);		CGI&	operator=(const CGI& copy);
		~CGI();

		bool	Run();
		void	ClearInheritedFds(int pipeOut);
		void	InitEnvpAndArgv();
		
		// bool	SendBodyToScript();
		bool	ReadOutputFromScript();
		
		void			RedirectIO();
		std::string		GetTmpOutputFile() const;
		TimerBenchmark	GetTimer() const;
		int				GetPipeOutFd();
		void			ClosePipeOut();
};
