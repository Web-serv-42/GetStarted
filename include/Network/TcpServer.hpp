/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:50:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 19:25:50 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "Network/Client.hpp"

#include <sstream>
#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

class TcpServer
{
	private:
		int	m_Port;
		int	m_ListenFd;

	public:
		TcpServer();
		TcpServer(int port);
		~TcpServer();

		bool	Setup();
		Client*	AcceptNewClient();
	
		int		GetPort() const;
		int		GetListenFd() const;		
};
