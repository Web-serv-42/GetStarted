/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wahmane <wahmane@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:35 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/17 19:42:25 by wahmane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "CGI/CGI.hpp"
#include "Core/HttpStatus.hpp"

#include "Parsing/ConfigResolver.hpp"

#include "HTTP/Request/Request.hpp"
#include "HTTP/Response/Response.hpp"
#include "Core/Timer.hpp"
#include "Session/SessionManager.hpp"

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

	STATE_RESPONSE_DONE,        // Response is fully built/file is fully read
    STATE_RESPONSE_IN_PROGRESS, // Still reading/writing large files (non-blocking)
    STATE_RESPONSE_ERROR,       // An error occurred, need to send an error page
    STATE_RESPONSE_DROP,

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
		Response 				m_Response;  // <-- Later: HTTP Response Builder
		ClientState				m_State;
		Routing					m_Routing;
		// Buffers to hold data if recv/send are interrupted (Non-blocking)
		std::string				m_ReadBuffer;
		std::string				m_WriteBuffer;

		// Content to send (static file/CGI tmpFile output).
		std::string				m_FileContentPath;
		int						m_ContentFileFd;
		// infos that we need for Routing
		std::string				m_LocalIp;
		int         			m_LocalPort;
		TimerBenchmark			m_Timer;
		Session*				m_Session;
		// Holds cookies to be sent out: Key -> Value (with optional settings like Path, Max-Age)
    	std::map<std::string, std::string>	m_OutboundCookies;

	public:
		Client();
		Client(int clientFd, struct sockaddr_storage m_ClientAddr);
		~Client();

		bool				ReadData();
		bool				SendData();
		
		Request&			GetRequest();
		const Request&		GetRequest() const;
		Response&			GetResponse();
		const Response&		GetResponse() const;
		
		// Response Phase
		int					ReadFileContent();
		int					PrepareWriteBuffer();
		int					ParseAndFinalizeCgiResponse();
		
		// Later I will add methods like:
   		void				BuildStaticResponse();
		void    HandleError(HttpStatusCode statusCode);
   		void				BuildStaticErrorResponse(HttpStatusCode code);

		int					GetClientFd() const;
		CGI*				GetCGI() const;
		ClientState			GetState() const;
		TimerBenchmark		GetTimer() const;
		Session*			GetSession();
		
		void				SetCGI(CGI* cgi);
		void				DeleteCGI();
		void				SetState(ClientState state);
		void				SetSession(Session* session);
		void				SetOutboundCookie(const std::string& name, const std::string& value, const std::string& attributes = "");

		void				DisplayClientInfo() const;
		
		const std::string&	GetRawRequestString() const;
		std::string&		GetRawRequestString();

		// for routing we need to know where did the request come from which port + host ? 
		void				SetLocalIp(const std::string& ip);
		void				SetLocalPort(int port);
		const std::string&	GetLocalIp() const;
		int					GetLocalPort() const;

		// ROUTING
		const Routing&		GetRouting() const;
		void				SetRouting(const Routing& routing);
};
