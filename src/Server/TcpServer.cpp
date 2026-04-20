/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:50:19 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/20 12:20:15 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/TcpServer.hpp"
#include "Core/Log.hpp"

#include <sstream>

#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

TcpServer::TcpServer()
{
}

TcpServer::TcpServer(int port) : m_Port(port)
{
}

TcpServer::~TcpServer()
{
}

bool			TcpServer::Setup()
{
	struct addrinfo		hints;
	struct addrinfo*	servinfo;

	int	status;
	int	sockfd;
	int	optval;

	// Prepare the target connection speceification
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags =AI_PASSIVE;

	// Get all set of connection possible
	std::stringstream	ss;
	ss << this->m_Port;
	if ((status = getaddrinfo(NULL, ss.str().c_str(), &hints, &servinfo)))
	{	
		ERROR_LOG("getaddrinfo: " + std::string(gai_strerror(status)));
		return (false);
	}

	// Loop through all connection and try to setup-bind it to a specifique port
	struct addrinfo*	current;
	for (current = servinfo; current != NULL; current = servinfo->ai_next)
	{
		// creates  an endpoint (network connection) for communication and returns a file descriptor
		if ((sockfd = socket(current->ai_family, current->ai_socktype, current->ai_protocol)) == -1)
			continue ;
		// I know there might be a previous connection in TIME_WAIT, but I want to take over this port anyway.
		if ((status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) == -1)
			TRACE_LOG("Failed to reuse the port");
		// Assigning a name to a socket
		if ((status = bind(sockfd, current->ai_addr, current->ai_addrlen) == 0))
			break ;
		if ((status = close(sockfd) == -1))
			ERROR_LOG("Failed to close sockfd endpoint");
	}
	// No result in servinfo is valid
	if (current == NULL)
	{
		ERROR_LOG("Failed to setup socket");
	}

	freeaddrinfo(servinfo);

	if ((status = listen(sockfd, SOMAXCONN)) == -1)
	{
		ERROR_LOG("Failed to listening / queue is full");
		return(false);
	}
	
	this->m_ListenFd = sockfd;
	return (true);
}

int		TcpServer::GetPort() const
{
	return (this->m_Port);
}

int		TcpServer::GetListenFd() const
{
	return (this->m_ListenFd);
}

