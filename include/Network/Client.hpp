/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:35 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/07 15:30:58 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "CGI/CGI.hpp"

#include <unistd.h>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class Client
{
	private:
		int						m_SocketFd;
		struct sockaddr_storage	m_ClientAddr;
		CGI*					m_CGI;
		// Request  m_Request;   <-- Later: HTTP Request Parser
		// Response m_Response;  <-- Later: HTTP Response Builder

		// Buffers to hold data if recv/send are interrupted (Non-blocking)
		std::string				m_ReadBuffer;
		std::string				m_WriteBuffer;
	public:
		Client();
		Client(int clientFd, struct sockaddr_storage m_ClientAddr);
		~Client();

		bool	ReadData();
		bool	SendData();
		bool	IsRequestComplete();
		bool	IsResponseSent();
		// Later I will add methods like:
		bool	HandleCGI();
   		void	BuildResponse();

		int		GetClientFd() const;
		CGI*	GetCGI() const;
		void	SetCGI(CGI* cgi);
		void	DisplayClientInfo() const;
};
