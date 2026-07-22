/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wahmane <wahmane@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:53 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/20 17:13:40 by wahmane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client/Client.hpp"

#define BUFFER_SIZE 131072

Client::Client()
    : m_SocketFd(-1), m_CGI(NULL), m_State(STATE_READING_REQUEST), m_LocalPort(-1)
{
}

Client::Client(int clientFd, struct sockaddr_storage clientAddr)
	: m_SocketFd(clientFd), m_ClientAddr(clientAddr), m_CGI(NULL), m_State(STATE_READING_REQUEST)
{
	this->DisplayClientInfo();
	this->m_Timer.Reset();
}

Client::~Client()
{
	if (this->m_SocketFd != -1)
	{	
		close(this->m_SocketFd);
	}
	delete	this->m_CGI;
}

const std::string& Client::GetRawRequestString() const 
{
	return this->m_ReadBuffer;
}

std::string& Client::GetRawRequestString()
{
	return this->m_ReadBuffer;
}

bool	Client::ReadData()
{
	ssize_t	receivedBytes;
	char	buffer[BUFFER_SIZE];

	//TODO Member 2: Max Body Size check
	receivedBytes = recv(this->m_SocketFd, buffer, BUFFER_SIZE, 0);
	if (receivedBytes == 0)
	{
		TRACE_LOG("Client closed the connection.");
		return (false);
	}
	else if (receivedBytes < 0)
	{
		ERROR_LOG("Socket Error: An error occurred when recv() data");
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


// bool    Client::SendData()
// {
//     ssize_t bytesSent;

//     if (this->m_WriteBuffer.empty())
//         return (true);
        
//     bytesSent = send(this->m_SocketFd, this->m_WriteBuffer.c_str(), this->m_WriteBuffer.length(), MSG_NOSIGNAL);
//     if (bytesSent < 0)
//     {
//         ERROR_LOG("Socket Error: An error occurred when send() data");
//         return (false);
//     }
    
//     this->m_WriteBuffer.erase(0, bytesSent);

//     if (this->m_WriteBuffer.empty() && 
//         (this->m_State == STATE_SENDING_FULL_RESPONSE || this->m_State == STATE_SENDING_ERROR_RESPONSE))
//     {
//         this->m_State = STATE_RESPONSE_SENT;
//     }

//     return (true);
// }
bool    Client::SendData()
{
    ssize_t bytesSent;

    if (this->m_State == STATE_RESPONSE_SENT) {
        return (false); 
    }

    if (this->m_WriteBuffer.empty())
    {
        if (this->m_State == STATE_SENDING_FULL_RESPONSE || this->m_State == STATE_SENDING_ERROR_RESPONSE)
        {
            this->m_State = STATE_RESPONSE_SENT;
            return (false);
        }
        return (true);
    }
        
    bytesSent = send(this->m_SocketFd, this->m_WriteBuffer.c_str(), this->m_WriteBuffer.length(), MSG_NOSIGNAL);
    if (bytesSent < 0)
    {
        ERROR_LOG("Socket Error: An error occurred when send() data");
        return (false);
    }
    
    this->m_WriteBuffer.erase(0, bytesSent);

    if (this->m_WriteBuffer.empty() && 
        (this->m_State == STATE_SENDING_FULL_RESPONSE || this->m_State == STATE_SENDING_ERROR_RESPONSE))
    {
        this->m_State = STATE_RESPONSE_SENT;
    }

    return (true);
}

void    Client::BuildStaticResponse()
{
    this->m_Response = Response(this->m_Routing, this->m_Request); 
    HttpStatusCode statusCode = this->m_Response.Run();

    if (statusCode != NORMAL)
    {
        this->m_Response.handleError(statusCode);
    }
    
    this->m_WriteBuffer = this->m_Response.getRawResponse();
    this->m_ReadBuffer.clear();

    if (!this->m_Response.getFilePath().empty()) {
        this->m_State = STATE_SENDING_HEADERS; 
    } else {
        this->m_State = STATE_SENDING_FULL_RESPONSE; 
    }
}

void Client::BuildStaticErrorResponse(HttpStatusCode code)
{
    std::string reason = GetHttpStatusReason(code);
    
    // Create an explicit error page body
    std::ostringstream body;
    body << "<html><head><title>" << code << " " << reason << "</title></head>"
        << "<body><center><h1>" << code << " " << reason << "</h1></center>"
        << "<hr><center>Webserv/1.0</center></body></html>";

    std::string html = body.str();
    std::ostringstream response;

    // Assemble dynamic status line and headers
    response << "HTTP/1.0 " << code << " " << reason << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << html.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << html;

    this->m_WriteBuffer = response.str();
}

// Returns: -1 on error, 0 on EOF, or positive bytes read
int Client::ReadFileContent()
{
	char    buffer[BUFFER_SIZE];
	int     bytesRead;

	bytesRead = read(this->m_ContentFileFd, buffer, BUFFER_SIZE);
	if (bytesRead < 0)
	{
		ERROR_LOG("File I/O Error: Failed to read from static file fd");
		return (-1);
	}
	if (bytesRead == 0)
	{
		return (0); 
	}
	
	// SAFE: Appends exactly bytesRead from the raw char array
	this->m_WriteBuffer.append(buffer, bytesRead);
	return (bytesRead);
}

// From here I read the response and i sent it [request first then body]
// or just the whole response if is it stored in memory : return status code


int Client::PrepareWriteBuffer()
{
    struct stat         fileInfo;
    std::stringstream   headerStream;

    if ((this->m_State == STATE_SENDING_ERROR_RESPONSE
        || this->m_State == STATE_SENDING_FULL_RESPONSE)
        && !this->m_WriteBuffer.empty())
    {
        return (0);
    }

    
    if (this->m_CGI != NULL)
    {
        this->m_FileContentPath = this->m_CGI->GetTmpOutputFile();
    }
    else
    {
        // this->m_FileContentPath = this->m_Routing.filePath;
        this->m_FileContentPath = this->m_Response.getFilePath();
    }
    
    if (this->m_State == STATE_SENDING_HEADERS)
    {
        if (stat(this->m_FileContentPath.c_str(), &fileInfo) != 0)
        {
            ERROR_LOG("File I/O Error: Could not find mock body file to measure size");
            return (HTTP_INTERNAL_SERVER_ERROR);
        }

        headerStream << "HTTP/1.0 200 OK\r\n"
        << "Content-Type: text/html\r\n" 
        << "Content-Length: " << fileInfo.st_size << "\r\n" 
        << "\r\n";
        this->m_WriteBuffer = headerStream.str();
        
        this->m_ContentFileFd = open(this->m_FileContentPath.c_str(), O_RDONLY);
        if (this->m_ContentFileFd == -1)
        {
            ERROR_LOG("File I/O Error: Could not open mock body file"); 
            return (HTTP_INTERNAL_SERVER_ERROR);
        }
        this->m_State = STATE_SENDING_BODY;
    }
    else if (this->m_State == STATE_SENDING_BODY)
    {
        if (this->m_WriteBuffer.empty())
        {
            int res = this->ReadFileContent();
            if (res == -1)
            {
                close(this->m_ContentFileFd);   
                return (HTTP_INTERNAL_SERVER_ERROR);
            }
            
            if (res == 0)
            {
                close(this->m_ContentFileFd);
                this->m_State = STATE_RESPONSE_SENT;
            }
        }
    }
    return (0);
}

// =========================================================================
// GETTERS, SETTERS & BOILERPLATE HELPERS
// =========================================================================

int	Client::GetClientFd() const
{
	return (this->m_SocketFd);
}

CGI*	Client::GetCGI() const
{
	return (this->m_CGI);
}

void	Client::SetCGI(CGI* cgi)
{
	this->m_CGI = cgi;
}

void	Client::DeleteCGI()
{
	delete this->m_CGI;
	this->m_CGI = NULL;
}

ClientState	Client::GetState() const
{
	return (this->m_State);
}

void	Client::SetState(ClientState state)
{
	this->m_State = state;
}


TimerBenchmark	Client::GetTimer() const
{
	return (this->m_Timer);
}

Session*	Client::GetSession()
{
	return (this->m_Session);
}

void		Client::SetSession(Session* session)
{
	this->m_Session = session;
}

void	Client::SetOutboundCookie(const std::string& name, const std::string& value, const std::string& attributes)
{
	std::string	fullValue = value;

	if (!attributes.empty())
	{
		fullValue += "; " + attributes;
	}
	this->m_OutboundCookies[name] = fullValue;
	// How Member 2 will use your system later:
	// for (std::map<std::string, std::string>::iterator it = m_OutboundCookies.begin(); it != m_OutboundCookies.end(); ++it)
	// {
  	//   response << "Set-Cookie: " << it->first << "=" << it->second << "\r\n";
	// }
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

Request& Client::GetRequest()
{
    return m_Request;
}

const Request& Client::GetRequest() const
{
    return m_Request;
}

Response& Client::GetResponse()
{
    return m_Response;
}

const Response& Client::GetResponse() const
{
    return m_Response;
}

void Client::SetLocalIp(const std::string& ip)
{
    m_LocalIp = ip;
}

void Client::SetLocalPort(int port)
{
    m_LocalPort = port;
}

const std::string& Client::GetLocalIp() const
{
    return m_LocalIp;
}

int Client::GetLocalPort() const
{
    return m_LocalPort;
}

// ROUTING
void	Client::SetRouting(const Routing& routing)
{
	m_Routing = routing;
}

const Routing&	Client::GetRouting() const
{
	return m_Routing;
}
