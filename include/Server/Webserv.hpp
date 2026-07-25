/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:00:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/02 16:36:37 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "Core/Timer.hpp"
#include "Network/TcpServer.hpp"
#include "Network/Multiplexer.hpp"
#include "Client/Client.hpp" 
#include "Client/ClientManager.hpp"
#include "CGI/CGI.hpp"
#include "CGI/CGIManager.hpp"
#include "Parsing/ConfigParser.hpp"
#include "Parsing/ConfigResolver.hpp"



#include <vector>
#include <map>

#include <csignal>

class Webserv
{
	private:
		volatile static bool	m_IsRunning;
		std::vector<TcpServer*>	m_Servers;
		Multiplexer				m_Polling;
		CGIManager				m_CGIManager;
		ClientManager			m_ClientManager;
		ConfigResolver*			m_Resolver;
		double					m_LastSessionCleanup;

	public:	
		Webserv();
		~Webserv();

		// bool		Init();
		bool 		Init(const ConfigTree& config);
		void		Run();
		void		Shutdown();

		static void	HandleSignals(int sigint);
		void		SetupSignals();

		bool		IsServerFd(int triggeredFd);
		TcpServer*	GetServerByFd(int serverFd);

		void		CheckSessionExpire();
};
