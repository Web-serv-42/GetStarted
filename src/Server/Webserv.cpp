/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:01:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/23 15:50:28 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"

Webserv::Webserv() : m_IsRunning(false) {}

Webserv::~Webserv()
{
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		delete this->m_Servers[i];
	}
}

bool	Webserv::Init()
{
	TRACE_LOG("Initializing Webserv Engine...");
	this->m_Polling.Init();
	// Parse config file
	// std::vector<int>	ports = this->m_Parser.getPorts();	// Real usage
	std::vector<int>	ports; ports.push_back(8080);		// For testing
	for (size_t i = 0; i < ports.size(); i++)
	{
		TcpServer*	server = new TcpServer(ports[i]);
		server->Setup();
		this->m_Servers.push_back(server);
		this->m_Polling.AddConnection(server->GetListenFd(), EPOLLIN);
	}
	INFO_LOG("Webserv successfully initialized.");
	return (true);
}

void	Webserv::Run()
{
	Timer::Init();
	int	numEvents = 0;
	this->m_IsRunning = true;
	INFO_LOG("Start listening for events...");

	// Server Loop
	while (this->m_IsRunning)
	{
		for (int i = 0; i < numEvents; i++)
		{
			// Check for new connection, data ..
			// is new client [add him]
			// existing client [handle data]
		}
		if (Timer::GetServerUptime() > 4.0)
			break ;
	}
	
}

void	Webserv::Shutdown()
{
	INFO_LOG("Shutting down Webserv Engine...");
    this->m_IsRunning = false;
	// Clean recources
}

bool	Webserv::IsServerFd(int fd)
{
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		if (fd == this->m_Servers[i]->GetListenFd())
			return (true);
	}
	return (false);
}

void	Webserv::AcceptNewConnection(int fd)
{

}

void	Webserv::HandleClientData(int fd)
{

}
