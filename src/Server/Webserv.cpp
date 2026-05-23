/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 13:01:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/23 12:20:36 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"
#include "Utils/utils.hpp"

// ======================= Engine =======================
Webserv::Webserv() : m_IsRunning(false) {}

Webserv::~Webserv()
{
	std::map<int, Client*>::iterator	it;
	for (it = this->m_Clients.begin(); it != this->m_Clients.end();)
	{
		Client*	client = it->second;
		it++;
		this->DisconnectClient(client);
	}
	for (size_t i = 0; i < this->m_Servers.size(); i++)
	{
		delete this->m_Servers[i];
	}
}

bool	Webserv::Init()
{
	INFO_LOG("Initializing Webserv Engine...");
	Timer::Init();
	this->m_Polling.Init();
	//TODO Memeber 3: Parse config file
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
			// Can be either server/client/cgiPipe Fd
			int	triggeredFd = this->m_Polling.GetEventFd(eventIndex);
			if (this->IsServerFd(triggeredFd))
			{
				this->ConnectClient(triggeredFd);
			}
			else if (this->IsCGIPipe(triggeredFd))
			{
				this->HandleCGI(triggeredFd, eventIndex);
			}
			else
			{
				this->ServeClient(triggeredFd, eventIndex);
			}
		}
		CheckCGITimeouts();
		// Shutdown Webserv after 10s
		if (Timer::GetServerUptime() > 10.0)
		{
			INFO_LOG("Stoping Webserv ...");
			break ;
		}
	}
}

void	Webserv::Shutdown()
{
	INFO_LOG("Shutting down Webserv Engine...");
	this->m_IsRunning = false;
	// Clean recources
}


// ======================= Client =======================
void	Webserv::ConnectClient(int serverFd)
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
	{
		this->m_Clients.erase(newClient->GetClientFd());
		delete	newClient;
		ERROR_LOG("Failed to connect to client");
		return;
	}
	SUCCESS_LOG("New client connected");
}

void	Webserv::ServeClient(int clientFd, int eventIndex)
{
	Client*	client = this->m_Clients[clientFd];

	// --- 1. CLIENT DROPS CONNECTION ---
	if (this->m_Polling.IsErrorFired(eventIndex))
	{
		this->DisconnectClient(client);
		return;
	}
	// --- 2. CLIENT SENT US DATA ---
	if (this->m_Polling.IsReadReady(eventIndex))
	{
		// Read BUFFER_SIZE of coming request
		if (client->ReadData() == false)
		{
			this->DisconnectClient(client);
			return;
		}
		if (this->ProcessRequest(client) != 200)
		{
			client->BuildErrorResponse();
			client->SetState(STATE_SENDING_HEADERS);
			// this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
		}
	}
	// --- 3. WE CAN SEND DATA TO CLIENT --- (Also in the same time with Read, this is why i'm using if)
	if (this->m_Polling.IsWriteReady(eventIndex))
	{
		client->PrepareWriteBuffer();
		// TRACE_LOG("Manage Client Response");
		if (client->SendData() == false)
		{
			this->DisconnectClient(client);
			return;
		}
		if (client->GetState() == STATE_RESPONSE_SENT)
		{
			//TODO: Not mandatory but it can be a keep-alive request [just reset the client for new request]
			client->SetState(STATE_READING_HEADERS);
			this->DisconnectClient(client);
		}
	}
}

int	Webserv::Routing(Client* client)
{
	//TODO Member 2: The Router (Logic Bridge)
	// Simple for now
	int	statusCode = client->ValidateRequestWithRouter();
	return (statusCode);
}

int	Webserv::ProcessRequest(Client* client)
{
	int	statusCode;

	// Parsing Headers first
	if (client->GetState() == STATE_READING_HEADERS)
	{
		//TODO Member 2: HttpRequest Hearders Parser
		statusCode = client->ProcessHeaders();
		if (statusCode != 200)
		{
			return (statusCode);
		}
		// Routing logic
		//TODO Member 1: Change State to STATE_ROUTING_INTERCEPTION
		if (client->GetState() == STATE_ROUTING_INTERCEPTION)
		{
			statusCode = this->Routing(client);
			if (statusCode != 200)
			{
				return (statusCode);
			}
			//TODO Member 1: Change State to STATE_PROCESSING Directly if no body found
			//TODO Member 1: Change State to STATE_READING_BODY if body found
		}
	}
	// Body Parsing
	if (client->GetState() == STATE_READING_BODY)
	{
		//TODO Member 2: HttpRequest Body Parser [The routing is already checked]
		statusCode = client->ProcessBody();
		if (statusCode != 200)
		{
			return (statusCode);
		}
		//TODO Member 1: Change State to STATE_PROCESSING
		// Executing Request
		if (client->GetState() == STATE_PROCESSING)
		{
			// Static file or CGI
			this->ExecuteRequest(client);
			//TODO Member 1: Change State to STATE_SENDING_HEADERS
		}
	}
	return (200); // No error accured, even still more to consume from request or is it aleady handled
}

void	Webserv::DisconnectClient(Client* client)
{
	// if (client != NULL) //? Extra safety check for NULL pointers
	// {
	//     this->m_Clients.erase(it);
	//     return;
	// }
	CGI*	cgi = client->GetCGI();
	// Safety: If the client was running a CGI, kill it and remove its pipes
	if (cgi && client->GetState() == STATE_WAITING_CGI)
	{
		DEBUG_LOG("Deatach Activated CGI");
		this->DetachCGI(cgi);
	}
	int	clientFd = client->GetClientFd();
	this->m_Polling.RemoveConnection(clientFd);
	// Clean up memory and map using the iterator)
	delete client;
	this->m_Clients.erase(clientFd);
	INFO_LOG("Client Disconnected Successfully");
}


// ======================= Request/Response =======================
void	Webserv::ExecuteRequest(Client* client)
{
	// At this point the whole request is processed, time to execute it	
	if (/* condition to check if it's a CGI request */ true)
	{
		client->SetState(STATE_WAITING_CGI);
		//TODO Member 2: CGI parametres input
		this->AttachCGI(client);
		// STOP HERE. Do not switch the client to EPOLLOUT yet.
		// Let epoll handle the pipes in the background.
	}
	else
	{
		// It's a static file request (e.g., index.html)
		// client->BuildResponse();
		client->SetState(STATE_SENDING_HEADERS);
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
	}
}

void	Webserv::BuildResponse(Client* client)
{
	//TODO Member 2: HttpResponse Builder
	//TODO Member 2: Bad HttpResponse Builder
	this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLIN);
}


// ======================= CGI =======================
void	Webserv::AttachCGI(Client* client)
{
	// --- FAKE ROUTER START ---
	// In the future, this comes from Member 3's logic.
	std::string interpreter = "/usr/bin/python3"; // Or path to cgi_tester
	std::string scriptPath = "./ect/generateHtmlPage.py";
	std::string	tmpFileBody = "./ect/response_1.tmp";
	std::string	tmpFileOutput = GenerateTmpFileName("cgi");
	bool		hasBody = true;

	std::vector<std::string> envVars;
	envVars.push_back("REQUEST_METHOD=POST");
	envVars.push_back("SERVER_PROTOCOL=HTTP/1.0");
	envVars.push_back("CONTENT_LENGTH=809"); // Length of requestBody
	envVars.push_back("CONTENT_TYPE=plain/text");
	envVars.push_back("SCRIPT_FILENAME=" + scriptPath);
	envVars.push_back("REDIRECT_STATUS=200"); // Required by python-cgi
	// --- FAKE ROUTER END ---

	CGI*	cgi = new CGI(interpreter, scriptPath, envVars, hasBody, tmpFileBody, tmpFileOutput);
	//TODO Track tmp file or fd so you can work with both static or CGI
	if (cgi->Run() == true)
	{
		client->SetCGI(cgi);
		client->SetState(STATE_WAITING_CGI);

		int	pipeOutFd = cgi->GetPipeOutFd();
		this->m_Polling.AddConnection(pipeOutFd, EPOLLIN);
		this->m_CgiFdToClient[pipeOutFd] = client;
	}
	else
	{
		ERROR_LOG("Failed to execute CGI");
		delete	cgi;
		client->SetCGI(NULL);
		//TODO Member 2 client->BuildErrorResponse(500); // You will need to implement this
		client->BuildErrorResponse();
		// Switch state so we can send an error immediately
		client->SetState(STATE_SENDING_HEADERS);
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
	}
}

void	Webserv::HandleCGI(int pipeFd, int eventIndex)
{
	//TODO Memeber 1: Timeout Timer
	Client*	client = this->m_CgiFdToClient[pipeFd];
	CGI*	cgi = client->GetCGI();

	if (this->m_Polling.IsErrorFired(eventIndex))
	{
		this->DisconnectClient(client);
		return;
	}
	// Read output from the CGI script via read()
	if (cgi->ReadOutputFromScript())
	{
		//TODO Member 2: HttpResponse Builder For CGI
		// Stop watching the read pipe so it doesn't trigger anymore
		this->DetachPipe(pipeFd);
		cgi->ClosePipeOut(); // Safely close and set to -1
		// 4. Wake the client socket back up in epoll to send the data
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
		INFO_LOG("CGI Terminated and Response Ready");
	}
}

void	Webserv::DetachPipe(int pipeFd)
{
	this->m_Polling.RemoveConnection(pipeFd);
	this->m_CgiFdToClient.erase(pipeFd);
}

void	Webserv::DetachCGI(CGI* cgi)
{
	int	pipeOutFd = cgi->GetPipeOutFd();

	if (pipeOutFd != -1)
	{
		this->DetachPipe(pipeOutFd);
		cgi->ClosePipeOut();
	}
}

void	Webserv::CheckCGITimeouts()
{
	// Iterate through all active clients/CGIs
    for (std::map<int, Client*>::iterator it = this->m_Clients.begin(); it != this->m_Clients.end(); ++it)
    {
		Client*	client = it->second;
		CGI*	cgi = client->GetCGI();
		if (cgi)
		{
			if (cgi->GetTimer().Elapsed() > TIMEOUT)
			{
				ERROR_LOG("CGI Timeout! Killing process");
				// 2. Build a 504 Gateway Timeout response // Status code parametr
                client->BuildErrorResponse();
				this->DisconnectClient(client);
				this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
			}
		}
	}
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
