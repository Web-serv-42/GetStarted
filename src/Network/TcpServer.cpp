/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:50:19 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 19:25:57 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Network/TcpServer.hpp"

TcpServer::TcpServer()
{
}

TcpServer::TcpServer(int port) : m_Port(port), m_ListenFd(-1)
{
}

TcpServer::~TcpServer()
{
	if (this->m_ListenFd != -1)
		close(this->m_ListenFd);
}

bool	TcpServer::Setup()
{
	struct addrinfo		hints;
	struct addrinfo*	servinfo;

	int	status;
	int	sockfd;
	int	optval = 1;

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
	
	// Store the ListenFd
	this->m_ListenFd = sockfd;
	SUCCESS_LOG("Listening on port: " + ss.str());
	return (true);
}


Client*	TcpServer::AcceptNewConnection()
{
	int						clientFd;
	struct sockaddr_storage	clientAddr;
	socklen_t				addrLen = sizeof(clientAddr);

	if ((clientFd = accept(this->m_ListenFd, (struct sockaddr*)&clientAddr, &addrLen)) == -1)
	{
		ERROR_LOG("Failed to accept client connection");
		return (NULL);
	}
	// What is this ???
	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	return (new Client(clientFd, clientAddr));
}

int	TcpServer::GetPort() const
{
	return (this->m_Port);
}

int	TcpServer::GetListenFd() const
{
	return (this->m_ListenFd);
}
