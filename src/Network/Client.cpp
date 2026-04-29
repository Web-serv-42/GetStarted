/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:53 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/25 19:23:52 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Network/Client.hpp"

#define BUFFER_SIZE 4096

Client::Client()
{
}

Client::Client(int clientFd, struct sockaddr_storage clientAddr)
	: m_SocketFd(clientFd), m_ClientAddr(clientAddr)
{
	this->DisplayClientInfo();
}

Client::~Client()
{
	close(this->m_SocketFd);
}

bool	Client::ReadData()
{
	ssize_t	receivedBytes;
	char	buffer[BUFFER_SIZE];

	memset((void*)&buffer, 0, sizeof(buffer));
	receivedBytes = recv(this->m_SocketFd, (void*)&buffer, sizeof(buffer), 0);
	if (receivedBytes == 0)
	{
		ERROR_LOG("Client closed the connection.");
		return (false);
	}
	else if (receivedBytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		ERROR_LOG("An error occurred when recv() data");
		return (false);
	}
	else
	{
		std::string receivedStr(buffer, receivedBytes);
		SUCCESS_LOG("Server received: " + receivedStr);
		this->m_ReadBuffer.append(buffer, receivedBytes); // Safe append!
		return (true);
	}
}

bool	Client::SendData()
{
	ssize_t	bytesSent;

	if (this->m_WriteBuffer.empty())
		return(true) ;
	bytesSent = send(this->m_SocketFd, this->m_WriteBuffer.c_str(), this->m_WriteBuffer.length(), MSG_NOSIGNAL);
	if (bytesSent < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		ERROR_LOG("An error occurred when send() data");
		return (false);
	}
	this->m_WriteBuffer.erase(0, bytesSent);
	return (true);
}

void	Client::BuildResponse()
{
	// A standard HTTP 200 OK response with some basic HTML
    std::string html = "<html><body><h1>Hello from Webserv Engine!</h1></body></html>";
    
    this->m_WriteBuffer = "HTTP/1.1 200 OK\r\n";
    this->m_WriteBuffer += "Content-Type: text/html\r\n";
    this->m_WriteBuffer += "Content-Length: 61\r\n"; // Length of the html string
    this->m_WriteBuffer += "\r\n"; // Empty line separating headers from body
    this->m_WriteBuffer += html;
    
    // Clear the read buffer so we are ready for the next request (Keep-Alive)
    this->m_ReadBuffer.clear();
}

bool	Client::IsRequestComplete()
{
	return (this->m_ReadBuffer.find("\r\n\r\n") != std::string::npos);
}

bool	Client::IsResponseSent()
{
	return (this->m_WriteBuffer.empty());
}

int	Client::GetClientFd() const
{
	return (this->m_SocketFd);
}

void	Client::DisplayClientInfo() const
{
	char 					str[INET6_ADDRSTRLEN];
	struct sockaddr_in*		ptr = NULL;
	struct sockaddr_in6*	ptr1 = NULL;

	// Print IP address of the new client
	if (this->m_ClientAddr.ss_family == AF_INET)
	{
		ptr = (struct sockaddr_in *) &this->m_ClientAddr;
		inet_ntop(AF_INET, &(ptr->sin_addr), str, sizeof(str));
	}
	else if (this->m_ClientAddr.ss_family == AF_INET6)
	{
		ptr1 = (struct sockaddr_in6 *) &this->m_ClientAddr;
		inet_ntop(AF_INET6, &(ptr1->sin6_addr), str, sizeof(str));
	}
	else
	{
		ptr = NULL;
		TRACE_LOG("Address family is neither AF_INET nor AF_INET6");
	}
	if (ptr) 
		INFO_LOG("Connection from client: " + std::string(str));
}
