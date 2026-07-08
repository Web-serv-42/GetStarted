/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:50:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/02 14:46:22 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "Client/Client.hpp"

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
		std::string	m_Host;
		int			m_Port;
		int			m_ListenFd;

	public:
		TcpServer();
		TcpServer(std::string host, int port);
		~TcpServer();

		bool	Setup();
		Client*	AcceptNewClient();
	
		int		GetPort() const;
		int		GetListenFd() const;		
};