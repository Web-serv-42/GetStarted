/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:35 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/03 23:04:52 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "CGI/CGI.hpp"
#include "../HTTP/Request/Request.hpp"

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
	STATE_EXECUTING,
	STATE_WAITING_CGI,
	//* Send Phase
	STATE_SENDING_ERROR_RESPONSE,
	STATE_SENDING_FULL_RESPONSE,
	STATE_SENDING_HEADERS,
	STATE_SENDING_BODY,
	STATE_RESPONSE_SENT,
	STATE_DISCONNECTED
};

class Client
{
	private:
		int						m_SocketFd;
		struct sockaddr_storage	m_ClientAddr;
		CGI*					m_CGI;
		Request  m_Request;   //<-- Later: HTTP Request Parser
		// Response m_Response;  <-- Later: HTTP Response Builder
		ClientState				m_State;
		// Buffers to hold data if recv/send are interrupted (Non-blocking)
		std::string				m_ReadBuffer;
		std::string				m_WriteBuffer;

		// Flags to trigger send()
		// bool					m_HeaderSent;
		// bool					m_BodySent;
		// Content to send (static file/CGI tmpFile output).
		std::string				m_FileContentPath;
		int						m_ContentFileFd;

	public:
		Client();
		Client(int clientFd, struct sockaddr_storage m_ClientAddr);
		~Client();

		bool	ReadData();
		bool	SendData();
		
		Request& GetRequest();
		const Request& GetRequest() const;
		
		// Entry point for processing request
		int	ProcessRequest();
		// Request Phase
		int	ProcessHeaders();
		int	ProcessBody();
		int	ValidateRequestWithRouter();
		// Response Phase
		int	ReadFileContent();
		int	PrepareWriteBuffer();
		int	ParseAndFinalizeCgiResponse();
		
		// Later I will add methods like:
   		void	BuildStaticResponse();
   		void	BuildStaticErrorResponse();

		int			GetClientFd() const;
		CGI*		GetCGI() const;
		void		SetCGI(CGI* cgi);
		void		DeleteCGI();
		ClientState	GetState() const;
		void		SetState(ClientState state);
		void		DisplayClientInfo() const;
		const std::string &GetRawRequestString() const;
};
