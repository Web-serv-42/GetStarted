/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:00:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/07 16:26:19 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "Core/Timer.hpp"
#include "Network/TcpServer.hpp"
#include "Network/Client.hpp" 
#include "Network/Multiplexer.hpp"
#include "CGI/CGI.hpp"

#include <vector>
#include <map>

class Webserv
{
	private:
		bool					m_IsRunning;
		// Vector is bad
		std::vector<TcpServer*>	m_Servers;
		std::map<int, Client*>	m_Clients;
		std::map<int, Client*>	m_CgiFdToClient;
		Multiplexer				m_Polling;

	public:
		Webserv();
		~Webserv();

		bool	Init();
		void	Run();
		void	Shutdown();

		void		AcceptNewClient(int serverFd);
		void		HandleClientData(int clientFd, int eventIndex);
		void		HandleRequest(Client* client);
		void		HandleResponse(Client* client);
		void		DisconnectClient(int clientFd);

		void		HandleNewCGI(Client* client);
		void		HandleExistingCGI(int pipeFd, int eventIndex);
		void		CleanupCGI(CGI* cgi);

		bool		IsServerFd(int triggeredFd);
		bool		IsCGIPipe(int triggeredFd);
		TcpServer*	GetServerByFd(int serverFd);
};
