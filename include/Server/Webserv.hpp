/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:00:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/23 12:10:11 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Core/Log.hpp"
#include "Core/Timer.hpp"
#include "Network/TcpServer.hpp"
#include "Network/Client.hpp" 
#include "Network/Multiplexer.hpp"
#include "CGI/CGI.hpp"

#include <vector>
#include <map>

#define TIMEOUT 5.0

class Webserv
{
	private:
		bool					m_IsRunning;
		// Vector is bad
		std::vector<TcpServer*>	m_Servers;
		std::map<int, Client*>	m_Clients;
		std::map<int, Client*>	m_CgiFdToClient;
		Multiplexer				m_Polling;

	public:
		Webserv();
		~Webserv();

		bool	Init();
		void	Run();
		void	Shutdown();

		void		ConnectClient(int serverFd);
		void		ServeClient(int clientFd, int eventIndex);
		void		DisconnectClient(Client* client);

		int			ProcessRequest(Client* client);
		int			Routing(Client* client);

		void		ExecuteRequest(Client* client);
		void		BuildResponse(Client* client);

		void		AttachCGI(Client* client);
		void		HandleCGI(int pipeFd, int eventIndex);
		void		DetachPipe(int pipeFd);
		void		DetachCGI(CGI* cgi);
		void		CheckCGITimeouts();


		bool		IsServerFd(int triggeredFd);
		bool		IsCGIPipe(int triggeredFd);
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
