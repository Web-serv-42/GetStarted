/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 19:03:47 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 11:54:03 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client/Client.hpp"
#include "Parsing/ConfigResolver.hpp"
#include "Network/Multiplexer.hpp"
#include "Network/TcpServer.hpp"
#include "CGI/CGIManager.hpp"
#include "Core/Timer.hpp"

#include <map>

#define CGI_TIMEOUT 5.0
#define CLIENT_TIMEOUT 5.0

class ClientManager
{
	private:
		std::map<int, Client*>	m_Clients;
		Multiplexer&			m_Polling;
		CGIManager&				m_CGIManager;
		// we would use this for the routing phase , to get the server and location
		ConfigResolver* 		m_Resolver;

	public:
		ClientManager(Multiplexer& poller, CGIManager& CGIManager);
		ClientManager(Multiplexer& poller, CGIManager& CGIManager,ConfigResolver * resolver);
		~ClientManager();

		void			ConnectClient(TcpServer* server);

		void			ServeClient(int clientFd, int eventIndex);
		HttpStatusCode	HandleInboundData(Client* client);
		void			DispatchResponse(Client* client);

		void			DisconnectClient(Client* client);
		void			CheckTimeouts(CGIManager& cgiManager);

		void			SetResolver(ConfigResolver *resolver);
		Client*			GetClient(int clientFd);
		void 			PrintRoutingInfo(Client* client);
};
