/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:01:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/01 15:59:39 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"
#include "../include/Parsing/ConfigParser.hpp"
#include "Utils/utils.hpp"
#include <vector>
#include <set>

// ======================= Engine =======================
Webserv::Webserv() :
			m_IsRunning(false),
			m_Polling(),
			m_CGIManager(m_Polling),
			m_ClientManager(m_Polling, m_CGIManager)
{
	
}

Webserv::~Webserv()
{
	
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		delete this->m_Servers[i];
	}
}

bool	Webserv::Init(const ConfigTree& config)
{
	INFO_LOG("Initializing Webserv Engine...");
	Timer::Init();
	this->m_Polling.Init();
	//TODO Memeber 3: Parse config file
	// for unique ports or we could use vector 
	std::vector<int> ports;

	for (size_t i = 0; i < config.servers.size(); ++i) 
    {
        std::map<std::string, std::string>::const_iterator it = config.servers[i].directives.find("listen");
        
        if (it != config.servers[i].directives.end()) 
        {
            std::string listenStr = it->second;
            int port = 0;

            // Handle the "IP:PORT" vs "PORT" rule (e.g., "127.0.0.1:8080" vs "8080")
            size_t colonPos = listenStr.find(':');
            if (colonPos != std::string::npos) {
                port = std::atoi(listenStr.substr(colonPos + 1).c_str());
            } else {
                port = std::atoi(listenStr.c_str());
            }

            ports.insert(port);
        }
    }

	// for (std::set<int>::iterator it = ports.begin(); it != ports.end(); ++it)
    // {
    //     int port = *it;
    //     TcpServer* server = new TcpServer(port);
        
    //     server->Setup(); // This should call socket(), bind(), and listen()
        
    //     this->m_Servers.push_back(server);
    //     this->m_Polling.AddConnection(server->GetListenFd(), EPOLLIN);
    // }
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
			// Can be either server/client/cgiPipe Fd
			int	triggeredFd = this->m_Polling.GetEventFd(eventIndex);
			if (this->IsServerFd(triggeredFd))
			{
				this->m_ClientManager.ConnectClient(this->GetServerByFd(triggeredFd));
			}
			else if (this->m_CGIManager.IsCGIPipe(triggeredFd))
			{
				this->m_CGIManager.HandleCGI(triggeredFd, eventIndex);
			}
			else
			{
				this->m_ClientManager.ServeClient(triggeredFd, eventIndex);
			}
		}
		this->m_CGIManager.CheckCGITimeouts();
		// Shutdown Webserv after 10s
		// if (Timer::GetServerUptime() > 10.0)
		// {
		// 	INFO_LOG("Stoping Webserv ...");
		// 	break ;
		// }
	}
}

void	Webserv::Shutdown()
{
	INFO_LOG("Shutting down Webserv Engine...");
	this->m_IsRunning = false;
	// Clean recources
}

// ======================= Helpers =======================
bool	Webserv::IsServerFd(int triggeredFd)
{
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		if (triggeredFd == this->m_Servers[i]->GetListenFd())
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
