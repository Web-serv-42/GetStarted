/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 19:03:47 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/03 23:30:43 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client/Client.hpp"
#include "../Parsing/ConfigResolver.hpp"
#include "Network/Multiplexer.hpp"
#include "Network/TcpServer.hpp"
#include "CGI/CGIManager.hpp"

#include <map>

class ClientManager
{
	private:
		std::map<int, Client*>	m_Clients;
		Multiplexer&			m_Polling;
		CGIManager&				m_CGIManager;
		ConfigResolver* 		m_Resolver;

	public:
		ClientManager(Multiplexer& poller, CGIManager& CGIManager);
		ClientManager(Multiplexer& poller, CGIManager& CGIManager,ConfigResolver * resolver);
		~ClientManager();

		void		ConnectClient(TcpServer* server);
		void		ServeClient(int clientFd, int eventIndex);
		void		ExecuteRequest(Client* client);
		void		DisconnectClient(Client* client);
		void		CheckClientTimeouts();
		void		SetResolver(ConfigResolver *resolver);
		Client*		GetClient(int clientFd);
		void 		PrintRoutingInfo(Client* client);
};
