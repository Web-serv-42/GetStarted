/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:00:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/01 15:43:04 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "../include/Parsing/ConfigParser.hpp"
#include "Core/Timer.hpp"
#include "Network/TcpServer.hpp"
#include "Network/Multiplexer.hpp"
#include "Client/Client.hpp" 
#include "Client/ClientManager.hpp"
#include "CGI/CGI.hpp"
#include "CGI/CGIManager.hpp"
#include "Parsing/Lexer.hpp"

#include <vector>
#include <map>


class Webserv
{
	private:
		bool					m_IsRunning;
		// Vector is bad
		std::vector<TcpServer*>	m_Servers;
		Multiplexer				m_Polling;
		CGIManager				m_CGIManager;
		ClientManager			m_ClientManager;

	public:
		Webserv();
		~Webserv();

		bool		Init(const ConfigTree& config);
		void		Run();
		void		Shutdown();

		bool		IsServerFd(int triggeredFd);
		TcpServer*	GetServerByFd(int serverFd);
};

// ================================== Webserv Life-Cycle ==================================
//	Init Webserv:
//		Parse Config file
//		Init Multiplexer 
//		Init TcpServers
//	 Track Clients: [always]
//	 	Connect/Disconnect Client 
//	 	Handle Client Request: 
//	 		Read Headers
//	 		Parse Headers
//	 		Router:
//				Method Check + Location Lookup + build environment_variables/parameters
//				Read Body if it exist:
//	 				Store as String [small] both Normal Request and CGI Request / large_body_error if Normal Request / Tmp_file if CGI Request [large] 
//	 			Normal Method [GET - POST - DELETE]
//					Deliver static content from disk + ...
//	 			CGI Script:
//	 				Read body if it exist:
//	 					stdin => [small body]
//	 					tmp_File => [large body]
//	 				Redirect body input to stdin pipe
//	 				Execute script
//	 				Redirect output to stdout pipe
//	 				chunk output ? tmp_file approach ?
//	 			Handle Error + error pages
//	 		Parse response
//	 		Build correct response
//	 		Handle Error + error pages
//	 	Clear Recources	
//	 Shutdown Webserv		
//	 	
