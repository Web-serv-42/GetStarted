/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:35 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/19 17:52:40 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "CGI/CGI.hpp"

#include <sstream>

#include <unistd.h>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

enum ClientState {
	//* Read Phase
	STATE_READING_HEADERS,
	STATE_ROUTING_INTERCEPTION,
	STATE_READING_BODY,
	//* Execute Phase
	STATE_PROCESSING,	   // Parsing request (No epoll interaction)
	STATE_WAITING_CGI,	  // Waiting for Python script (Webserv handles pipes)
	//* Send Phase
	STATE_READY_TO_SEND,	// Needs EPOLLOUT to send response
	STATE_SENDING_HEADERS,
	STATE_SENDING_BODY,
	STATE_RESPONSE_SENT,
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

		// Flags to trigger send()
		// bool					m_HeaderSent;
		// bool					m_BodySent;
		// Content to send (static file/CGI tmpFile output)
		int						m_ContentFileFd;

	public:
		Client();
		Client(int clientFd, struct sockaddr_storage m_ClientAddr);
		~Client();

		bool	ReadData();
		bool	SendData();

		// Request Phase
		int	ProcessHeaders();
		int	ProcessBody();
		int	ValidateRequestWithRouter();
		// Response Phase
		int	ReadFileContent();
		int	PrepareWriteBuffer();
		int	ParseAndFinalizeCgiResponse();
		
		// Later I will add methods like:
   		void	BuildResponse();
   		void	BuildErrorResponse();

		int			GetClientFd() const;
		CGI*		GetCGI() const;
		void		SetCGI(CGI* cgi);
		ClientState	GetState() const;
		void		SetState(ClientState state);
		void		DisplayClientInfo() const;
};
