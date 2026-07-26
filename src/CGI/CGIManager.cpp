/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIManager.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 18:37:40 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/26 10:45:08 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI/CGIManager.hpp"
#include "Core/HttpStatus.hpp"

CGIManager::CGIManager(Multiplexer& poller) : m_Polling(poller)
{
	
}

CGIManager::~CGIManager()
{
}

HttpStatusCode      CGIManager::AttachCGI(Client* client)
{
    if (!client)
        return (HTTP_INTERNAL_SERVER_ERROR);

    const Request& request = client->GetRequest();
    const Routing& routing = client->GetRouting();
    size_t limitInMB = routing.location->client_max_body_size;
    size_t limitInBytes = limitInMB * 1024 * 1024;
    std::vector<std::string> envVars;

    // Check limit (assuming 0 means unlimited)
    if (limitInMB > 0 && request.GetBodyReceived() > limitInBytes)
    {
        return (HTTP_PAYLOAD_TOO_LARGE);
    }
    // 1. Verify the CGI interpreter binary exists and can be executed
    if (access(routing.cgiInterpreter.c_str(), F_OK | X_OK) != 0)
    {
        ERROR_LOG("CGI Error: Interpreter not found or not executable: " + routing.cgiInterpreter);
        return (HTTP_BAD_GATEWAY);
    }
    std::string interpreter = routing.cgiInterpreter;

    // 2. Verify the actual target script exists and is readable
    if (access(routing.filePath.c_str(), F_OK | R_OK) != 0)
    {
        ERROR_LOG("CGI Error: Script file not found or unreadable: " + routing.filePath);
        return (HTTP_BAD_GATEWAY);
    }
    std::string fullPath = routing.filePath;
    std::string scriptPath = "./"; 
    std::string scriptName = fullPath;

    size_t lastSlashPos = fullPath.find_last_of('/');
    if (lastSlashPos != std::string::npos)
    {
        scriptPath = fullPath.substr(0, lastSlashPos + 1); // , "./cgi-bin/"
        scriptName = fullPath.substr(lastSlashPos + 1);    // , "info.php"
    }

    // 3. Only validate the body path if the request actually contains a body payload!
    bool hasBody = (request.GetContentLength() > 0); 
    std::string tmpFileBody = "";
    if (hasBody)
    {
        tmpFileBody = request.GetBodyFilePath();
        if (access(tmpFileBody.c_str(), F_OK | R_OK) != 0)
        {
            ERROR_LOG("CGI Error: Input body file missing or unreadable: " + tmpFileBody);
            return (HTTP_BAD_GATEWAY);
        }
    }

    // The file hasn't been created yet—let your cgi->Run() loop handle creating it safely.
    std::string tmpFileOutput = "./tmp/" + GenerateTmpFileName("cgi_out");

    // 5. Build Environment Block
    envVars.push_back("REQUEST_METHOD=" + request.GetMethodString());
    envVars.push_back("SERVER_PROTOCOL=HTTP/1.0");
    if (hasBody)
    {
        envVars.push_back("CONTENT_LENGTH=" + request.GetHeader("content-length"));
        envVars.push_back("CONTENT_TYPE=" + request.GetHeader("content-type"));
    }
    envVars.push_back("SCRIPT_FILENAME=" + scriptName);
    if (!request.GetQuery().empty())
        envVars.push_back("QUERY_STRING=" + request.GetQuery());    
    envVars.push_back("REDIRECT_STATUS=200");

    // 6. Instantiation and execution fork sequence
    CGI*    cgi = new CGI(interpreter, scriptPath, scriptName, envVars, hasBody, tmpFileBody, tmpFileOutput);
    if (cgi->Run() == true)
    {
        client->SetCGI(cgi);
        client->SetState(STATE_WAITING_CGI);

        int pipeOutFd = cgi->GetPipeOutFd();
        this->m_Polling.AddConnection(pipeOutFd, EPOLLIN);
        this->m_CgiFdToClient[pipeOutFd] = client;
    }
    else
    {
        ERROR_LOG("CGI Error: Runtime fork/execve processing failure occurred inside cgi->Run()");
        delete  cgi;
        client->SetCGI(NULL);
        return (HTTP_BAD_GATEWAY);
    }
    return (NORMAL);
}

void		CGIManager::HandleCGI(int pipeFd, int eventIndex)
{
	DEBUG_LOG("Start handling CGI...");
	//TODO Memeber 1: Timeout Timer
	Client*	client = this->m_CgiFdToClient[pipeFd];
	CGI*	cgi = client->GetCGI();

	if (this->m_Polling.IsErrorFired(eventIndex))
	{
		ERROR_LOG("CGI Error: CGI pipe error or hangup detected");
		this->DetachCGI(cgi); // Remove pipe from epoll and map
		client->DeleteCGI();  // Fire destructor to clean up process/files
		
        client->BuildErrorResponse(HTTP_INTERNAL_SERVER_ERROR);
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
		return;
	}
	// Read output from the CGI script via read()
    int status = cgi->ReadOutputFromScript();
	if (status == 1)
	{
		this->DetachPipe(pipeFd);
		cgi->ClosePipeOut();
		// 4. Wake the client socket back up in epoll to send the data
		client->SetState(STATE_SENDING_HEADERS);
		this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
		INFO_LOG("CGI Terminated and Response Ready");
	}
    else if (status == -1) // CRASH / FAILURE
    {
        this->DetachPipe(pipeFd);
        cgi->ClosePipeOut();
        // Use your existing error builder so it serves standard error pages
		client->SetState(STATE_SENDING_CGI_ERROR_RESPONSE);
        this->m_Polling.ModifyConnection(client->GetClientFd(), EPOLLOUT);
        ERROR_LOG("CGI Failed - Sending 502 Bad Gateway");
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
