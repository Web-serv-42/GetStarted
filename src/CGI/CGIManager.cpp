/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIManager.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 18:37:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/06/10 19:37:38 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGIManager.hpp"

CGIManager::CGIManager(Multiplexer& poller) : m_Polling(poller)
{
	
}

CGIManager::~CGIManager()
{
}

void		CGIManager::AttachCGI(Client* client)
{
	if (!client)
		return;
	// // --- FAKE ROUTER START (PYTHON3 EDITION)  ---
	// // In the future, this comes from Member 1's logic.
	// std::string interpreter = "/usr/bin/python3"; // Or path to cgi_tester
	// std::string scriptPath = "./cgi-bin/";
	// std::string scriptName = "generateHtmlPage.py";
	// std::string	tmpFileBody = "./cgi-bin/cgiBody_1.tmp";
	// std::string	tmpFileOutput = GenerateTmpFileName("cgi");
	// bool		hasBody = true;

	// std::vector<std::string> envVars;
	// envVars.push_back("REQUEST_METHOD=POST");
	// envVars.push_back("SERVER_PROTOCOL=HTTP/1.0");
	// envVars.push_back("CONTENT_LENGTH=809"); // Length of requestBody
	// envVars.push_back("CONTENT_TYPE=plain/text");
	// envVars.push_back("SCRIPT_FILENAME=" + scriptName);
	// envVars.push_back("REDIRECT_STATUS=200"); // Required by python-cgi
	// // --- FAKE ROUTER END ---

	// --- FAKE ROUTER START (PHP-CGI EDITION) ---
    // On Ubuntu, the CGI flavor is always 'php-cgi', not regular 'php'
    // std::string interpreter = "/usr/bin/php-cgi"; 
    // std::string scriptPath = "./cgi-bin/";
    // std::string scriptName = "info.php";
    // std::string	tmpFileBody = "./cgi-bin/cgiBody_1.tmp";
    // std::string tmpFileOutput = GenerateTmpFileName("cgi");
    // bool        hasBody = true;

    // std::vector<std::string> envVars;
    // envVars.push_back("REQUEST_METHOD=POST");
    // envVars.push_back("SERVER_PROTOCOL=HTTP/1.0");
	// envVars.push_back("CONTENT_LENGTH=809"); // Length of requestBody
	// envVars.push_back("CONTENT_TYPE=plain/text");
    // envVars.push_back("SCRIPT_FILENAME=" + scriptName);
    // envVars.push_back("REDIRECT_STATUS=200"); 
    // --- FAKE ROUTER END ---

	const Request& request = client->GetRequest();
	const Routing& routing = client->GetRouting();

	std::string interpreter = routing.cgiInterpreter;
	// Split the physical path into Directory (for chdir) and Name (for execve)
    std::string fullPath = routing.filePath;
    std::string scriptPath = "./"; // fallback
    std::string scriptName = fullPath;

	size_t lastSlashPos = fullPath.find_last_of('/');
    if (lastSlashPos != std::string::npos)
    {
        scriptPath = fullPath.substr(0, lastSlashPos + 1); // e.g., "./www/cgi-bin/"
        scriptName = fullPath.substr(lastSlashPos + 1);    // e.g., "info.php"
    }

    // Now grab the body path directly from the parsed request
    std::string tmpFileBody = request.GetBodyFilePath(); 
    std::string tmpFileOutput = GenerateTmpFileName("cgi_out");
    bool hasBody = (request.GetContentLength() > 0); // Cleaner check based on your Request object
	std::cout << "LEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEENGTH: " << request.GetContentLength() << std::endl;
    std::vector<std::string> envVars;
    envVars.push_back("REQUEST_METHOD=" + request.GetMethodString());
    envVars.push_back("SERVER_PROTOCOL=HTTP/1.0");
	if (hasBody)
    {
        envVars.push_back("CONTENT_LENGTH=" + request.GetHeader("content-length"));
        // envVars.push_back("CONTENT_TYPE=" + request.GetHeader("content-type"));
		envVars.push_back("CONTENT_TYPE=plain/text");
    }
    envVars.push_back("SCRIPT_FILENAME=" + scriptName);
	if (!request.GetQuery().empty())
    	envVars.push_back("QUERY_STRING=" + request.GetQuery());	
    envVars.push_back("REDIRECT_STATUS=200");

	CGI*	cgi = new CGI(interpreter, scriptPath, scriptName, envVars, hasBody, tmpFileBody, tmpFileOutput);
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
		client->BuildStaticErrorResponse(/*500*/);
		// Switch state so we can send an error immediately
		client->SetState(STATE_SENDING_ERROR_RESPONSE);
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
	}
}

void		CGIManager::HandleCGI(int pipeFd, int eventIndex)
{
	DEBUG_LOG("Start handling CGI...");
	//TODO Memeber 1: Timeout Timer
	Client*	client = this->m_CgiFdToClient[pipeFd];
	CGI*	cgi = client->GetCGI();

	if (this->m_Polling.IsErrorFired(eventIndex))
	{
		client->BuildStaticErrorResponse();
		client->SetState(STATE_SENDING_ERROR_RESPONSE);
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
		client->SetState(STATE_SENDING_HEADERS); ///////////
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
		INFO_LOG("CGI Terminated and Response Ready");
	}
}

void		CGIManager::DetachPipe(int pipeFd)
{
	this->m_Polling.RemoveConnection(pipeFd);
	this->m_CgiFdToClient.erase(pipeFd);
}

void		CGIManager::DetachCGI(CGI* cgi)
{
	if (!cgi)
		return;
	int	pipeOutFd = cgi->GetPipeOutFd();

	if (pipeOutFd != -1)
	{
		this->DetachPipe(pipeOutFd);
		cgi->ClosePipeOut();
	}
}

bool	CGIManager::IsCGIPipe(int triggeredFd)
{
	if (this->m_CgiFdToClient.find(triggeredFd) != this->m_CgiFdToClient.end())
		return (true);
	return (false);
}

void		CGIManager::CheckCGITimeouts()
{
	// Iterate through all active clients/CGIs
    for (std::map<int, Client*>::iterator it = this->m_CgiFdToClient.begin(); it != this->m_CgiFdToClient.end();)
    {
		Client*	client = it->second;
		CGI*	cgi = client->GetCGI();
		if (cgi && client->GetState() == STATE_WAITING_CGI)
		{
			if (cgi->GetTimer().Elapsed() > TIMEOUT)
			{
				std::map<int, Client*>::iterator next = it;
        		++next;
				ERROR_LOG("CGI Timeout! Killing process");
				// 1. Delete the CGI and clean up the pipes safely
                this->DetachCGI(cgi);
                client->DeleteCGI();

                // 2. Build a 504 Gateway Timeout response
                // You will need to implement this so BuildStaticErrorResponse takes an arg
                client->BuildStaticErrorResponse(/* 504 */); 

                // 3. Switch the client state to send the error
                client->SetState(STATE_SENDING_ERROR_RESPONSE);
                this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
                
                // Let the next iteration of the epoll loop handle sending 
                // the data via ServeClient(). DO NOT disconnect here.
				it = next;
        		continue;
			}
		}
		++it;
	}
}
