/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wahmane <wahmane@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 19:03:36 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/17 19:45:39 by wahmane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client/ClientManager.hpp"
#include "HTTP/Request/Request.hpp"
#include "Parsing/ConfigResolver.hpp"
#include "Parsing/RequestParser.hpp"
#include "Core/HttpStatus.hpp"

ClientManager::ClientManager(Multiplexer& poller, CGIManager& CGIManager) : m_Polling(poller), m_CGIManager(CGIManager)
{
}

ClientManager::ClientManager(Multiplexer& poller, CGIManager& CGIManager,ConfigResolver * resolver) : m_Polling(poller), m_CGIManager(CGIManager), m_Resolver(resolver)
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
		ERROR_LOG("Connection error: TCP server dosen't exist");
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
		ERROR_LOG("Connection error: Failed to connect to client");
		return;
	}
	SUCCESS_LOG("New client connected");
}

void ClientManager::PrintRoutingInfo(Client* client)
{
    const Request& request = client->GetRequest();
    const Routing& routing = client->GetRouting();

    std::string host = request.GetHeader("host");

    size_t colon = host.find(':');
    if (colon != std::string::npos)
        host.erase(colon);

    std::cout << "\n";
    std::cout << "=============== ROUTING ===============\n";

    std::cout << "Socket IP      : " << client->GetLocalIp() << "\n";
    std::cout << "Socket Port    : " << client->GetLocalPort() << "\n";
    std::cout << "Host Header    : " << host << "\n";
    std::cout << "URI            : " << request.GetPath() << "\n";
    std::cout << "Method         : " << request.GetMethod() << "\n";

    // --- ADDED: Parsed Cookies View ---
    const std::map<std::string, std::string>& cookies = request.GetCookies();
    if (!cookies.empty())
    {
        std::cout << "Cookies        :\n";
        for (std::map<std::string, std::string>::const_iterator cit = cookies.begin(); cit != cookies.end(); ++cit)
        {
            std::cout << "    " << cit->first << " = " << cit->second << "\n";
        }
    }

    if (routing.server)
    {
        std::cout << "Server Name    : "
                  << routing.server->server_name << "\n";
    }
    else
    {
        std::cout << "Server         : NOT FOUND\n";
    }

    if (routing.location)
    {
        std::cout << "Location       : "
                  << routing.location->path << "\n";

        std::cout << "Root           : "
                  << routing.location->root << "\n";

        std::cout << "File Path  : "
                  << routing.filePath << "\n";

        std::cout << "Index          : "
                  << routing.location->index << "\n";

        std::cout << "Autoindex      : "
                  << (routing.location->autoindex ? "ON" : "OFF") << "\n";

        std::cout << "Body Limit     : "
                  << routing.location->client_max_body_size
                  << " bytes\n";

        std::cout << "Upload Path    : "
                  << routing.location->upload_file << "\n";

        std::cout << "Allowed Methods: ";

        for (size_t i = 0; i < routing.location->allow_methods.size(); ++i)
        {
            if (i)
                std::cout << ", ";

            std::cout << routing.location->allow_methods[i];
        }

        std::cout << "\n";

        if (routing.location->return_directive.first != 0)
        {
            std::cout << "Redirect       : "
                      << routing.location->return_directive.first
                      << " -> "
                      << routing.location->return_directive.second
                      << "\n";
        }

        if (!routing.location->cgis.empty())
        {
            std::cout << "CGI:\n";

            std::map<std::string, std::string>::const_iterator it;

            for (it = routing.location->cgis.begin();
                 it != routing.location->cgis.end();
                 ++it)
            {
                std::cout << "    "
                          << it->first
                          << " -> "
                          << it->second
                          << "\n";
            }
        }

        if (!routing.location->error_pages.empty())
        {
            std::cout << "Error Pages:\n";

            std::map<int, std::string>::const_iterator it;

            for (it = routing.location->error_pages.begin();
                 it != routing.location->error_pages.end();
                 ++it)
            {
                std::cout << "    "
                          << it->first
                          << " -> "
                          << it->second
                          << "\n";
            }
        }
    }
    else
    {
        std::cout << "Location       : NOT FOUND\n";
    }

    std::cout << "=======================================\n\n";
}

void PrintParsedRequest(const Request& req) 
{
    std::cout << "\n\033[1;35m" << std::string(60, '=') << "\033[0m\n";
    std::cout << "\033[1;33m[+] HTTP REQUEST PARSER OUTPUT [+]\033[0m\n";
    std::cout << "\033[1;35m" << std::string(60, '-') << "\033[0m\n\n";

    // 1. Resolve Method String
    std::string methodStr = "UNKNOWN";
    if (req.GetMethod() == HTTP_GET) methodStr = "GET";
    else if (req.GetMethod() == HTTP_POST) methodStr = "POST";
    else if (req.GetMethod() == HTTP_DELETE) methodStr = "DELETE";

    // 2. Print Request Line Data
    std::cout << "\033[1;32m[REQUEST LINE]\033[0m\n";
    std::cout << "  Method  : \033[0;36m" << methodStr << "\033[0m\n";
    std::cout << "  Path    : \033[0;36m" << req.GetPath() << "\033[0m\n";
    std::cout << "  Query   : \033[0;36m" << (req.GetQuery().empty() ? "(none)" : req.GetQuery()) << "\033[0m\n";
    std::cout << "  Version : \033[0;36m" << (req.GetVesrion().empty() ? "(none)" : req.GetVesrion()) << "\033[0m\n\n";

    // 3. Print Headers Map
    std::cout << "\033[1;32m[HEADERS]\033[0m\n";
    const std::map<std::string, std::string>& headers = req.GetHeaders();
    if (headers.empty()) {
        std::cout << "  (none)\n";
    } else {
        std::map<std::string, std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); ++it) {
            std::cout << "  " << it->first << " : \033[0;36m" << it->second << "\033[0m\n";
        }
    }
    std::cout << "\n";

    // --- ADDED: 3.5 Print Cookies Map Block ---
    std::cout << "\033[1;32m[COOKIES]\033[0m\n";
    const std::map<std::string, std::string>& cookies = req.GetCookies();
    if (cookies.empty()) {
        std::cout << "  \033[0;90m(none)\033[0m\n";
    } else {
        std::map<std::string, std::string>::const_iterator cit;
        for (cit = cookies.begin(); cit != cookies.end(); ++cit) {
            // Using Magenta (\033[0;35m) for cookie keys to make them pop vs standard headers
            std::cout << "  \033[0;35m" << cit->first << "\033[0m = \033[0;36m" << cit->second << "\033[0m\n";
        }
    }
    std::cout << "\n";

    // 4. Print Body Data
    std::cout << "\033[1;32m[BODY]\033[0m\n";
    std::cout << "  Expected Content-Length : " << req.GetContentLength() << " bytes\n";
    std::cout << "  Body File Path          :" << req.GetBodyFilePath() << "\n";
    std::cout << "  Body File Fd            :\n\033[0;36m";
    (req.GetBodyFd() == -1)
        ? (std::cout << "(no FD)")
        : (std::cout << req.GetBodyFd());
    std::cout << "\033[0m\n\n";

    // 5. Print State & Errors
    std::cout << "\033[1;32m[INTERNAL STATE]\033[0m\n";
    if (req.GetErrorCode() != 0) {
        std::cout << "  Error Code : \033[1;31m" << req.GetErrorCode() << " (Parsing Failed!)\033[0m\n";
    } else {
        std::cout << "  Error Code : 0 (No Errors)\n";
    }
    
    std::cout << "  Is Ready?  : ";
    if (req.GetState() == PARSE_COMPLETE) {
        std::cout << "\033[1;32mYES (Ready for Router)\033[0m\n";
    } else {
        std::cout << "\033[1;33mNO (Waiting for more data from epoll...)\033[0m\n";
    }

    std::cout << "\033[1;35m" << std::string(60, '=') << "\033[0m\n\n";
}

void ClientManager::ServeClient(int clientFd, int eventIndex)
{
	HttpStatusCode     statusCode;
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
		DEBUG_LOG("Reading chunk from socket...");
		statusCode = this->HandleInboundData(client);
		if (statusCode == DROP_CONNECTION)
		{
			this->DisconnectClient(client);
			return ;
		}
		else if (statusCode != NORMAL)
		{
			std::cout << ("PARSING ERROR DETECTED !!!!!!!!!!!!!!!!!!!!!!!!!!!! ") << statusCode << std::endl ;
			// client->BuildStaticErrorResponse(statusCode); // Here !!!!!!!!!!!!
			client->HandleError(statusCode);
			// client->GetResponse().generateErrorResponse(statusCode);
			// client->SetState(STATE_SENDING_ERROR_RESPONSE);
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
			client->SetState(STATE_READING_REQUEST);
			this->DisconnectClient(client);
		}
	}
}

HttpStatusCode	ClientManager::HandleInboundData(Client* client)
{
	if (client->ReadData() == false)
		return (DROP_CONNECTION); // Signal an immediate connection drop to the layer above

	bool is_request_fully_parsed = RequestParser::Parse(client->GetRequest(), client->GetRawRequestString());
	if (!is_request_fully_parsed)
	{
		DEBUG_LOG("Request incomplete. Yielding execution back to epoll loop.");
		return (NORMAL);  // 0 explicitly means: "Nothing to do, keep reading"
	}
	Request&	request = client->GetRequest();
	PrintParsedRequest(request);

	this->TrackSession(client, request);

	HttpStatusCode	parserError = request.GetErrorCode();
	if (parserError != 0)
		return (parserError); // could be  400 or 500 

	// -------------------------------------------------
	// Resolve routing.
	// -------------------------------------------------
	std::string host = request.GetHeader("host");

	size_t colon = host.find(':');
	if (colon != std::string::npos)
		host.erase(colon);

	Routing routing = m_Resolver->ResolveRequest(
		client->GetLocalIp(),
		client->GetLocalPort(),
		host,
		request.GetPath());

	client->SetRouting(routing);
	PrintRoutingInfo(client);

	client->SetState(STATE_EXECUTING); 
	this->DispatchResponse(client);

	return (NORMAL); // Execution kicked off safely, no errors to report
}

void	ClientManager::TrackSession(Client* client, Request& request)
{
	// Manage Client Session
	std::string	sessionId = request.GetCookie("webserv_sid");
	Session*	currentSession = NULL;
	
	if (!sessionId.empty())
		currentSession = this->m_SessionManager.GetSession(sessionId);
	if (currentSession == NULL)
	{
		currentSession = this->m_SessionManager.CreateSession();
		// Save some dummy session details to demonstrate tracking to evaluators
        currentSession->data["user_tier"] = "guest_account";
        currentSession->data["visit_count"] = "1";
		
		// Queue up the Set-Cookie header so the outbound pipeline drops it down the socket
		client->SetOutboundCookie("webserv_sid", currentSession->sessionId, "Path=/; HttpOnly");
		DEBUG_LOG("Created new server session ID: " + currentSession->sessionId);
	}
	else
	{
		int visits = std::atoi(currentSession->data["visit_count"].c_str());
        std::ostringstream oss;
        oss << (visits + 1);
        currentSession->data["visit_count"] = oss.str();
        
        DEBUG_LOG("Welcome back session ID: " + currentSession->sessionId + " | Visits: " + oss.str());
	}
	// Attach the session reference directly to the client object so your application handles it
    client->SetSession(currentSession);
}

void	ClientManager::DispatchResponse(Client* client)
{
	// At this point the whole request is processed, time to execute it	
	const Routing& routing = client->GetRouting();
	HttpStatusCode			statusCode = NORMAL;

	if (!routing.cgiInterpreter.empty())
	{
		client->SetState(STATE_WAITING_CGI);
		//TODO Member 2: CGI parametres input
		statusCode = this->m_CGIManager.AttachCGI(client);
		if (statusCode != NORMAL)
		{
			client->HandleError(statusCode);
			this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
		}
		// STOP HERE. Do not switch the client to EPOLLOUT yet [CGI runing in background successfuly give it some time].
		// Let epoll handle the pipes in the background.
	}
	else
	{
		// It's a static file request (e.g., index.html)
		client->BuildStaticResponse(); // Here again !!!!!!!!!
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
	}
}

void		ClientManager::DisconnectClient(Client* client)
{
	if (!client)
		return ;
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

void	ClientManager::CheckTimeouts(CGIManager& cgiManager)
{
	for (std::map<int, Client*>::iterator it = this->m_Clients.begin(); it != this->m_Clients.end();)
	{
		Client* client = it->second;
		
		// Advance iterator safely BEFORE any potential deletion or state modification
		std::map<int, Client*>::iterator next = it;
		++next;

		if (client)
		{
			// CASE 1: Client is actively waiting on a CGI script
			if (client->GetState() == STATE_WAITING_CGI)
			{
				CGI* cgi = client->GetCGI();
				if (cgi && cgi->GetTimer().Elapsed() > CGI_TIMEOUT)
				{
					ERROR_LOG("CGI Error: CGI Timeout! Killing process");
					
					// Safe to clear pipes because we aren't iterating over the CGI map!
					cgiManager.DetachCGI(cgi);
					client->DeleteCGI();

					// Set up the timeout response wrapper
					client->HandleError(HTTP_GATEWAY_TIMEOUT); 
					client->SetState(STATE_SENDING_ERROR_RESPONSE);
					this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
				}
			}
			// CASE 2: Client is just sitting idle (Inbound/Outbound standard traffic)
			// else if (client->GetState() == STATE_READING_REQUEST)
			else if (client->GetRequest().GetState() == PARSE_HEADERS)
			{
				if (client->GetTimer().Elapsed() > CLIENT_TIMEOUT)
				{
					ERROR_LOG("Client Error: Client inactivity timeout reached! Dropping connection");
					// Set up the timeout response wrapper
					client->HandleError(HTTP_REQUEST_TIMEOUT); 
					client->SetState(STATE_SENDING_ERROR_RESPONSE);
					this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
				}
			}
		}
		it = next;
	}
}

Client*		ClientManager::GetClient(int clientFd)
{
	return (this->m_Clients[clientFd]);
}

void ClientManager::SetResolver(ConfigResolver* resolver)
{
	m_Resolver = resolver;
}
