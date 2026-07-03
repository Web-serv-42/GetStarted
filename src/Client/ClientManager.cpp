/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 19:03:36 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/03 23:31:03 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client/ClientManager.hpp"
#include "../../include/HTTP/Request/Request.hpp"
#include "../../include/Parsing/ConfigResolver.hpp"

ClientManager::ClientManager(Multiplexer& poller, CGIManager& CGIManager) : m_Polling(poller), m_CGIManager(CGIManager)
{

}

ClientManager::~ClientManager()
{
	std::map<int, Client*>::iterator	it;
	for (it = this->m_Clients.begin(); it != this->m_Clients.end();)
	{
		Client*	client = it->second;
		it++;
		this->DisconnectClient(client);
	}
}

void		ClientManager::ConnectClient(TcpServer*	server)
{
	bool	isAdded;

	//1. Safety check for server pointer
	if (!server)
	{
		ERROR_LOG("TCP server dosen't exist");
		return;
	}
	// 2. Tell the server to accept the connection
	Client*		newClient = server->AcceptNewClient();

	if (!newClient)
		return;
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


void ClientManager::ServeClient(int clientFd, int eventIndex)
{
    int     statusCode;
    std::map<int, Client*>::iterator it = m_Clients.find(clientFd);

    if (it == m_Clients.end())
        return;
    Client* client = it->second;

    DEBUG_LOG("Serving Client ...");
    
    // --- 1. CLIENT DROPS CONNECTION ---
    if (this->m_Polling.IsErrorFired(eventIndex))
    {
        this->DisconnectClient(client);
        return;
    }
    
    // --- 2. CLIENT SENT US DATA ---
    if (this->m_Polling.IsReadReady(eventIndex))
    {
        DEBUG_LOG("Reading ...");
        
        // Read BUFFER_SIZE of coming request
        if (client->ReadData() == false)
        {
            this->DisconnectClient(client);
            return;
        }

        // =========================================================================
        // VISUALIZER BLOCK: Isolate and display the incoming request
        // =========================================================================
        // NOTE: Replace `GetRawRequestString()` with your actual getter method.
        std::string rawRequest = client->GetRawRequestString(); 
        
        if (!rawRequest.empty()) 
        {
            std::cout << "\n\033[1;36m" << std::string(60, '=') << "\033[0m\n";
            std::cout << "\033[1;32m[+] INCOMING HTTP REQUEST (FD: " << clientFd << ") [+]\033[0m\n";
            std::cout << "\033[1;36m" << std::string(60, '-') << "\033[0m\n";
            
            std::cout << "\033[0;37m" << rawRequest << "\033[0m"; 
            if (rawRequest[rawRequest.length() - 1] != '\n') std::cout << "\n";
            
            std::cout << "\033[1;36m" << std::string(60, '=') << "\033[0m\n\n";
        }
        // =========================================================================

        statusCode = client->ProcessRequest();

        if (statusCode == 200)
            {
                const Request& request = client->GetRequest();

                std::string host = request.GetHeader("Host");

                size_t colon = host.find(':');

                if (colon != std::string::npos)
                    host.erase(colon);

                int port = 8080; // TODO: replace with client's listening port.

                const ServerConfig* server =
                    m_Resolver->GetServerBy_Port_Host(port, host);

                const LocationConfig* location = NULL;

                if (server)
                {
                    location = m_Resolver->GetLocationBy_Server_Uri(
                        *server,
                        request.GetPath()
                    );
                }

                std::cout << "\n";
                std::cout << "============= ROUTING =============\n";
                std::cout << "Method   : " << request.GetMethod() << "\n";
                std::cout << "Host     : " << host << "\n";
                std::cout << "URI      : " << request.GetPath() << "\n";

                if (server)
                {
                    std::cout << "Server   : FOUND\n";

                    std::map<std::string,
                            std::vector<std::string> >::const_iterator it;

                    it = server->directives.find("server_name");

                    if (it != server->directives.end())
                    {
                        std::cout << "Names    : ";

                        for (size_t i = 0; i < it->second.size(); ++i)
                        {
                            if (i)
                                std::cout << ", ";

                            std::cout << it->second[i];
                        }

                        std::cout << "\n";
                    }
                }
                else
                {
                    std::cout << "Server   : NOT FOUND\n";
                }

                if (location)
                    std::cout << "Location : " << location->path << "\n";
                else
                    std::cout << "Location : /\n";

                std::cout << "===================================\n\n";
            }

        if (statusCode == 200 && client->GetState() == STATE_EXECUTING)
        {
            //TODO: Execute request and track status code
            this->ExecuteRequest(client);
        }
        else if (statusCode != 200)
        {
            client->BuildStaticErrorResponse();
            client->SetState(STATE_SENDING_ERROR_RESPONSE);
            this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
        }
    }
    
    // --- 3. WE CAN SEND DATA TO CLIENT --- 
    if (this->m_Polling.IsWriteReady(eventIndex))
    {
        DEBUG_LOG("Sending ...");
        client->PrepareWriteBuffer();
        
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


void	ClientManager::ExecuteRequest(Client* client)
{
	// At this point the whole request is processed, time to execute it	
	if (/* condition to check if it's a CGI request */ true)
	{
		client->SetState(STATE_WAITING_CGI);
		//TODO Member 2: CGI parametres input
		this->m_CGIManager.AttachCGI(client);
		// STOP HERE. Do not switch the client to EPOLLOUT yet.
		// Let epoll handle the pipes in the background.
	}
	else
	{
		// It's a static file request (e.g., index.html)
		client->BuildStaticResponse();
		client->SetState(STATE_SENDING_FULL_RESPONSE);
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
	}
}

void		ClientManager::DisconnectClient(Client* client)
{
	//TODO: To disconnet client you must cal its destructor that chckthe state of exesting CGI and detach it 
	//TODO then delete the client object 
	// if (client != NULL) //? Extra safety check for NULL pointers
	// {
	//     this->m_Clients.erase(it);
	//     return;
	// }
	CGI*	cgi = client->GetCGI();
	// Safety: If the client was running a CGI, kill it and remove its pipes
	if (cgi)
	{
		DEBUG_LOG("Deatach Activated CGI");
		this->m_CGIManager.DetachCGI(cgi);
	}
	int	clientFd = client->GetClientFd();
	this->m_Polling.RemoveConnection(clientFd);
	// Clean up memory and map using the iterator)
	delete client;
	this->m_Clients.erase(clientFd);
	INFO_LOG("Client Disconnected Successfully");
}

void		ClientManager::CheckClientTimeouts()
{
	
}

Client*		ClientManager::GetClient(int clientFd)
{
	return (this->m_Clients[clientFd]);
}

void ClientManager::SetResolver(ConfigResolver* resolver)
{
    m_Resolver = resolver;
}

