/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:35 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/10 15:45:15 by abnsila          ###   ########.fr       */
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

enum ClientState {
	STATE_READING_REQUEST,  // Waiting for EPOLLIN
	STATE_PROCESSING,	   // Parsing request (No epoll interaction)
	STATE_WAITING_CGI,	  // Waiting for Python script (Webserv handles pipes)
	STATE_READY_TO_SEND,	// Needs EPOLLOUT to send response
	STATE_DISCONNECTED	  // Needs to be cleaned up
};

class Client
{
	private:
		int						m_SocketFd;
		struct sockaddr_storage	m_ClientAddr;
		CGI*					m_CGI;
		// Request  m_Request;   <-- Later: HTTP Request Parser
		// Response m_Response;  <-- Later: HTTP Response Builder
		ClientState				m_State;
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

		int			GetClientFd() const;
		CGI*		GetCGI() const;
		void		SetCGI(CGI* cgi);
		ClientState	GetState() const;
		void		SetState(ClientState state);
		void		DisplayClientInfo() const;
};
