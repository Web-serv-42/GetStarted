/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:01:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/09 21:19:03 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"

Webserv::Webserv() : m_IsRunning(false) {}

Webserv::~Webserv()
{
	for (size_t i = 0; i < this->m_Clients.size(); i++)
	{
		this->DisconnectClient(this->m_Clients[i]->GetClientFd()) ;
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
			// Shutdown Webserv after 10s
			if (Timer::GetServerUptime() > 10.0)
				break ;
			// Can be either server/client/cgiPipe Fd
			int	triggeredFd = this->m_Polling.GetEventFd(eventIndex);
			if (this->IsServerFd(triggeredFd))
			{
				this->AcceptNewClient(triggeredFd);
			}
			else if (this->IsCGIPipe(triggeredFd))
			{
				this->HandleExistingCGI(triggeredFd, eventIndex);
			}
			else
			{
				this->HandleClientData(triggeredFd, eventIndex);
			}
		}
	}
}

void	Webserv::Shutdown()
{
	INFO_LOG("Shutting down Webserv Engine...");
	this->m_IsRunning = false;
	// Clean recources
}

void	Webserv::AcceptNewClient(int serverFd)
{
	bool	isAdded;

	// 1. Find which server was triggered
	TcpServer*	server = this->GetServerByFd(serverFd);
	// 2. Tell the server to accept the connection
	Client*		newClient = server->AcceptNewClient();

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

	// --- 1. CLIENT SENT US DATA ---
	if (this->m_Polling.IsReadReady(eventIndex))
	{
		// TRACE_LOG("Manage Client Request");
		if (client->ReadData() == false)
		{
			this->DisconnectClient(clientFd);
			return;
		}
		this->HandleRequest(client);
	}
	// --- 2. WE CAN SEND DATA TO CLIENT --- (Also in the same time with Read, this is why i'm using if)
	if (this->m_Polling.IsWriteReady(eventIndex))
	{
		// TRACE_LOG("Manage Client Response");
		if (client->SendData() == false)
		{
			this->DisconnectClient(clientFd);
			return;
		}
		this->HandleResponse(client);
	}
}

void	Webserv::HandleRequest(Client* client)
{
	if (client->IsRequestComplete())
	{
		// Member 2: HttpRequest Parser
		// Member 3: The Router (Logic Bridge)
		// Member 2: HttpResponse Builder
		if (/* condition to check if it's a CGI request */ true) 
        {
            this->HandleNewCGI(client);
            // STOP HERE. Do not switch the client to EPOLLOUT yet.
            // Let epoll handle the pipes in the background.
        }
        else
        {
            // It's a static file request (e.g., index.html)
            client->BuildResponse();
            this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
        }
	}
}

void	Webserv::HandleResponse(Client* client)
{
	if (client->IsResponseSent())
	{
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLIN);
	}
}

void	Webserv::HandleNewCGI(Client* client)
{
	// --- FAKE ROUTER START ---
	// In the future, this comes from Member 3's logic.
	std::string interpreter = "/usr/bin/python3"; // Or path to cgi_tester
	std::string scriptPath = "./ect/script.py";
	std::string requestBody = "user=Abdellah";

	std::vector<std::string> envVars;
	envVars.push_back("REQUEST_METHOD=POST");
	envVars.push_back("SERVER_PROTOCOL=HTTP/1.0");
	envVars.push_back("CONTENT_LENGTH=15"); // Length of requestBody
	envVars.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");
	envVars.push_back("SCRIPT_FILENAME=" + scriptPath);
	envVars.push_back("REDIRECT_STATUS=200"); // Required by python-cgi
	// --- FAKE ROUTER END ---

	CGI*	cgi = new CGI(interpreter, scriptPath, envVars, requestBody);

	if (cgi->Run() == true)
	{
		client->SetCGI(cgi);

		int	pipeInFd = cgi->GetPipeInFd();
		int	pipeOutFd = cgi->GetPipeOutFd();

		this->m_Polling.AddConnection(pipeInFd, EPOLLOUT);
		this->m_Polling.AddConnection(pipeOutFd, EPOLLIN);

		this->m_CgiFdToClient[pipeInFd] = client;
		this->m_CgiFdToClient[pipeOutFd] = client;
	}
}

void	Webserv::HandleExistingCGI(int pipeFd, int eventIndex)
{
	// TODO: Timer
	Client*	client = this->m_CgiFdToClient[pipeFd];
	CGI*	cgi = client->GetCGI();

	if (this->m_Polling.IsWriteReady(eventIndex))
	{
		// Send request body to the CGI script via write()
		if (cgi->SendBodyToScript())
		{
			// Stop watching the write pipe so it doesn't trigger anymore
			this->m_Polling.RemoveConnection(pipeFd);
			this->m_CgiFdToClient.erase(pipeFd);
			close(pipeFd);
		}
	}
	else if (this->m_Polling.IsReadReady(eventIndex))
	{
		// Read output from the CGI script via read()
		if (cgi->ReadOutputFromScript())
		{
			// It's Better not to call waitpid here inside the abstract engine
			// Pass the CGI output to Member 2's Response Builder

			// Clean up maps and delete the CGI object
			this->m_Polling.RemoveConnection(pipeFd);
			this->m_CgiFdToClient.erase(pipeFd);
			close(pipeFd);
			INFO_LOG("CGI Terminated and Response Ready");
		}
	}
}

void	Webserv::CleanupCGI(CGI* cgi)
{
	int	pipeInFd = cgi->GetPipeInFd();
	int	pipeOutFd = cgi->GetPipeOutFd();
	
	this->m_Polling.RemoveConnection(pipeInFd);
	this->m_Polling.RemoveConnection(pipeOutFd);
	
	this->m_CgiFdToClient.erase(pipeInFd);
	this->m_CgiFdToClient.erase(pipeOutFd);
}

void	Webserv::DisconnectClient(int clientFd)
{
	//TODO: CGI output not displayed ??????????
	Client* client = this->m_Clients[clientFd];
	CGI*	cgi = client->GetCGI();

	// Safety: If the client was running a CGI, kill it and remove its pipes
	if (cgi)
	{
		DEBUG_LOG("CGI Activated");
		this->CleanupCGI(cgi);
	}
	this->m_Polling.RemoveConnection(clientFd);
	// Free the memory (this also calls close(m_SocketFd) form the destructor)
	delete this->m_Clients[clientFd];
	// Remove the dangling pointer from the map
	this->m_Clients.erase(clientFd);
	INFO_LOG("Client Disconnected Succefuly");
}

bool	Webserv::IsServerFd(int triggeredFd)
{
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		if (triggeredFd == this->m_Servers[i]->GetListenFd())
			return (true);
	}
	return (false);
}

bool	Webserv::IsCGIPipe(int triggeredFd)
{
	if (this->m_CgiFdToClient.find(triggeredFd) != this->m_CgiFdToClient.end())
		return (true);
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
