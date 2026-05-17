/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 21:23:42 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/18 00:15:49 by abnsila          ###   ########.fr       */
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

enum BodyStorage
{
	BODY_IN_STRING,
	BODY_IN_FILE
};

class CGI
{
	private:
		std::string					m_Interpreter;
		std::string					m_ScriptPath;
		std::vector<std::string>	m_EnvVars;

		BodyStorage					m_BodyStorage;
		std::string					m_RequestBody;
		std::string					m_TmpBodyFile;
		std::string					m_TmpOutputFile;

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
		CGI(std::string interpreter, std::string scriptPath, std::vector<std::string> envVars, std::string bodyOrFile, BodyStorage mode);
		CGI&	operator=(const CGI& copy);
		~CGI();

		bool	Run();
		void	ClearInheritedFds(int pipeIn, int pipeOut);
		void	InitEnvpAndArgv();
		
		bool	SendBodyToScript();
		bool	ReadOutputFromScript();
		
		bool	SetupPipes();
		int		GetPipeInFd();
		int		GetPipeOutFd();
		void	ClosePipeIn();
		void	ClosePipeOut();

		BodyStorage	GetBodyStorage() const;

};
