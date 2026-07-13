/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:35 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 12:06:17 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "CGI/CGI.hpp"
#include "HTTP/Request/Request.hpp"
#include "Parsing/ConfigResolver.hpp"
#include "Core/HttpStatus.hpp"
#include "Core/Timer.hpp"

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
	STATE_READING_REQUEST,
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
		Request  				m_Request;   //<-- Later: HTTP Request Parser
		// Response m_Response;  <-- Later: HTTP Response Builder
		ClientState				m_State;
		Routing					m_Routing;
		// Buffers to hold data if recv/send are interrupted (Non-blocking)
		std::string				m_ReadBuffer;
		std::string				m_WriteBuffer;

		// Content to send (static file/CGI tmpFile output).
		std::string				m_FileContentPath;
		int						m_ContentFileFd;
		// infos that we need for Routing
		std::string m_LocalIp;
		int         m_LocalPort;
		TimerBenchmark			m_Timer;

	public:
		Client();
		Client(int clientFd, struct sockaddr_storage m_ClientAddr);
		~Client();

		bool				ReadData();
		bool				SendData();
		
		Request&			GetRequest();
		const Request&		GetRequest() const;
		
		// Response Phase
		int					ReadFileContent();
		int					PrepareWriteBuffer();
		int					ParseAndFinalizeCgiResponse();
		
		// Later I will add methods like:
   		void				BuildStaticResponse();
   		void				BuildStaticErrorResponse(HttpStatusCode code);

		int					GetClientFd() const;
		CGI*				GetCGI() const;
		void				SetCGI(CGI* cgi);
		void				DeleteCGI();
		ClientState			GetState() const;
		void				SetState(ClientState state);
		TimerBenchmark		GetTimer() const;
		void				DisplayClientInfo() const;
		
		const std::string&	GetRawRequestString() const;
		std::string&		GetRawRequestString();

		// for routing we need to know where did the request come from which port + host ? 
		void				SetLocalIp(const std::string& ip);
		void				SetLocalPort(int port);
		const std::string&	GetLocalIp() const;
		int					GetLocalPort() const;

		// ROUTING
		void				SetRouting(const Routing& routing);
		const Routing&		GetRouting() const;
};
