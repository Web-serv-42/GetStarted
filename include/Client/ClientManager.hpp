/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 19:03:47 by abnsila           #+#    #+#             */
/*   Updated: 2026/06/10 19:13:59 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client/Client.hpp"
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

	public:
		ClientManager(Multiplexer& poller, CGIManager& CGIManager);
		~ClientManager();

		void		ConnectClient(TcpServer* server);
		void		ServeClient(int clientFd, int eventIndex);
		void		ExecuteRequest(Client* client);
		void		DisconnectClient(Client* client);
		void		CheckClientTimeouts();

		Client*		GetClient(int clientFd);
};
