/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:00:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 18:12:12 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "Core/Timer.hpp"
#include "Network/TcpServer.hpp"
#include "Network/Client.hpp" 
#include "Network/Multiplexer.hpp"

#include "vector"
#include "map"

class Webserv
{
	private:
		bool					m_IsRunning;
		std::vector<TcpServer*>	m_Servers;
		std::map<int, Client*>	m_Clients;
		Multiplexer				m_Polling;

	public:
		Webserv();
		~Webserv();

		bool	Init();
		void	Run();
		void	Shutdown();

		void		AcceptNewConnection(int serverFd);
		void		HandleClientData(int clientFd, int eventIndex);

		bool		IsServerFd(int serverFd);
		TcpServer*	GetServerByFd(int serverFd);
};
