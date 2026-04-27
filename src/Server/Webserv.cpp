/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:01:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/04/27 16:56:25 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"

Webserv::Webserv() : m_IsRunning(false) {}

Webserv::~Webserv()
{
	for (size_t i = 0; i < this->m_Clients.size(); i++)
	{
		delete this->m_Clients[i];
	}
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		delete this->m_Servers[i];
	}
}

bool	Webserv::Init()
{
	TRACE_LOG("Initializing Webserv Engine...");
	Timer::Init();
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
	int	numEvents = 0;
	this->m_IsRunning = true;
	INFO_LOG("Start listening for events...");

	// Server Loop
	while (this->m_IsRunning)
	{
		numEvents = this->m_Polling.WaitEvents();
		for (int eventIndex = 0; eventIndex < numEvents; eventIndex++)
		{
			// Can be either client/server Fd
			int			triggeredFd = this->m_Polling.GetEventFd(eventIndex);
			if (this->IsServerFd(triggeredFd))
			{
				this->AcceptNewConnection(triggeredFd);
			}
			else
			{
				TRACE_LOG("Manage Client Request/Response");
				this->HandleClientData(triggeredFd, eventIndex);
			}
		}
		// Shutdown Webserv after 10s
		// if (Timer::GetServerUptime() > 10.0)
		// 	break ;
	}
}

void	Webserv::Shutdown()
{
	INFO_LOG("Shutting down Webserv Engine...");
    this->m_IsRunning = false;
	// Clean recources
}

void	Webserv::AcceptNewConnection(int serverFd)
{
	bool	isAdded;

	// 1. Find which server was triggered
	TcpServer*	server = this->GetServerByFd(serverFd);
	// 2. Tell the server to accept the connection
	Client*		newClient = server->AcceptNewConnection();
	
	// 3. Store the client in our Engine's memory
	this->m_Clients[newClient->GetClientFd()] = newClient;
	// 4. Tell the Multiplexer to watch this new client for incoming data
	isAdded = this->m_Polling.AddConnection(newClient->GetClientFd(), EPOLLIN);
	if (!isAdded)
		return;
	SUCCESS_LOG("New client connected");
}

void	Webserv::HandleClientData(int clientFd, int eventIndex)
{
	Client*	client = this->m_Clients[clientFd];
	(void)client;
	// --- 1. CLIENT SENT US DATA ---
	if (this->m_Polling.IsReadReady(eventIndex))
	{

	}
	// --- 2. WE CAN SEND DATA TO CLIENT ---
	else if (this->m_Polling.IsWriteReady(eventIndex))
	{

	}
}

bool	Webserv::IsServerFd(int serverFd)
{
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		if (serverFd == this->m_Servers[i]->GetListenFd())
			return (true);
	}
	return (false);
}

TcpServer*	Webserv::GetServerByFd(int serverFd)
{
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		if (serverFd == this->m_Servers[i]->GetListenFd())
			return (this->m_Servers[i]);
	}
	return (NULL);
}
