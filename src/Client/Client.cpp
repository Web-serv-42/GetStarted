/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:57:53 by abnsila           #+#    #+#             */
/*   Updated: 2026/06/10 19:20:49 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client/Client.hpp"

#define BUFFER_SIZE 4096

Client::Client()
{
}

Client::Client(int clientFd, struct sockaddr_storage clientAddr)
	: m_SocketFd(clientFd), m_ClientAddr(clientAddr), m_CGI(NULL), m_State(STATE_READING_HEADERS)
{
	this->DisplayClientInfo();
}

Client::~Client()
{
	if (this->m_SocketFd != -1)
	{	
		close(this->m_SocketFd);
	}
	delete	this->m_CGI;
}

bool	Client::ReadData()
{
	// Check Client Timeout
	ssize_t	receivedBytes;
	char	buffer[BUFFER_SIZE];

	//TODO Member 2: Max Body Size check
	receivedBytes = recv(this->m_SocketFd, (void*)&buffer, BUFFER_SIZE, 0);
	if (receivedBytes == 0)
	{
		TRACE_LOG("Client closed the connection.");
		return (false);
	}
	else if (receivedBytes < 0)
	{
		ERROR_LOG("An error occurred when recv() data");
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

bool	Client::SendData()
{
	ssize_t	bytesSent;

	
	// The response can be stored into a file
	if (this->m_WriteBuffer.empty())
		return(true) ;
	bytesSent = send(this->m_SocketFd, this->m_WriteBuffer.c_str(), this->m_WriteBuffer.length(), MSG_NOSIGNAL);
	if (bytesSent < 0)
	{
		ERROR_LOG("An error occurred when send() data");
		return (false);
	}
	this->m_WriteBuffer.erase(0, bytesSent);
	std::stringstream	ss;
	ss << bytesSent;
	//TODO Data sent, need static file and CGI logic, need parsing cheks
	return (true);
}

void	Client::BuildResponse()
{
	// A standard HTTP 200 OK response with some basic HTML
	std::string html = "<html><body><h1>Hello from Webserv Engine!</h1></body></html>";
	
	this->m_WriteBuffer = "HTTP/1.0 200 OK\r\n";
	this->m_WriteBuffer += "Content-Type: text/html\r\n";
	this->m_WriteBuffer += "Content-Length: 61\r\n"; // Length of the html string
	this->m_WriteBuffer += "\r\n"; // Empty line separating headers from body
	this->m_WriteBuffer += html;
	
	// Clear the read buffer so we are ready for the next request (Keep-Alive)
	this->m_ReadBuffer.clear();
}

void	Client::BuildErrorResponse()
{
	// A standard HTTP 200 OK response with some basic HTML
	std::string html = "<html><body><h1>Error</h1></body></html>";
	
	this->m_WriteBuffer = "HTTP/1.0 500 KO\r\n";
	this->m_WriteBuffer += "Content-Type: text/html\r\n";
	this->m_WriteBuffer += "Content-Length: 40\r\n"; // Length of the html string
	this->m_WriteBuffer += "\r\n"; // Empty line separating headers from body
	this->m_WriteBuffer += html;

	// Clear the read buffer so we are ready for the next request (Keep-Alive)
	this->m_ReadBuffer.clear();
}

int	Client::ProcessHeaders()
{
	this->m_State = STATE_ROUTING_INTERCEPTION;
	return (200);
}

int	Client::ProcessBody()
{
	this->m_State = STATE_EXECUTING;
	return (200);
}

int	Client::ValidateRequestWithRouter()
{
	this->m_State = STATE_READING_BODY;
	return (200);
}

int	Client::ParseAndFinalizeCgiResponse()
{
	return (200);
}

int	Client::ProcessRequest()
{
	int	statusCode;

	// Parsing Headers first
	if (this->m_State == STATE_READING_HEADERS)
	{
		//TODO Member 2: HttpRequest Hearders Parser
		statusCode = this->ProcessHeaders();
		if (statusCode != 200)
		{
			return (statusCode);
		}
		// Routing logic
		//TODO Member 1: Change State to STATE_ROUTING_INTERCEPTION
		if (this->m_State == STATE_ROUTING_INTERCEPTION)
		{
			statusCode = this->ValidateRequestWithRouter();
			if (statusCode != 200)
			{
				return (statusCode);
			}
			//TODO Member 1: Change State to STATE_EXECUTING Directly if no body found
			//TODO Member 1: Change State to STATE_READING_BODY if body found
		}
	}
	// Body Parsing
	if (this->m_State == STATE_READING_BODY)
	{
		//TODO Member 2: HttpRequest Body Parser [The routing is already checked]
		statusCode = this->ProcessBody();
		if (statusCode != 200)
		{
			return (statusCode);
		}
		//TODO Member 1: Change State to STATE_EXECUTING
		// Executing Request is done by ClientManager after parsing both Headers and Body
	}
	return (200); // No error accured, even still more to consume from request or is it aleady handled
}

// Returns: -1 on error, 0 on EOF, or positive bytes read
int Client::ReadFileContent()
{
	char    buffer[BUFFER_SIZE];
	int     bytesRead;

	bytesRead = read(this->m_ContentFileFd, buffer, BUFFER_SIZE);
	if (bytesRead < 0)
	{
		ERROR_LOG("Failed to read from static file fd");
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

int Client::PrepareWriteBuffer()
{
	//TODO Get tmp file or fd from response builder, so i can work with both static or CGI
	// Just for testing static file request
	// thism_FileContentPath = "./ect/body_4096_byte.tmp";
	struct stat 		fileInfo;
	std::stringstream	headerStream;

	// TODO:  Error responses are build separetly (:
	if (this->m_State == STATE_SENDING_ERROR_RESPONSE && !this->m_WriteBuffer.empty())
	{
		this->m_State = STATE_RESPONSE_SENT;
		return (200);
	}
	this->m_FileContentPath = this->m_CGI->GetTmpOutputFile();
	// 1. Initial trigger point from ExecuteRequest
	if (this->m_State == STATE_SENDING_HEADERS)
	{
		// Dynamically measure the exact file footprint on disk
		if (stat(this->m_FileContentPath.c_str(), &fileInfo) != 0)
		{
			ERROR_LOG("Could not find mock body file to measure size");
			return (500);
		}

		headerStream << "HTTP/1.0 200 OK\r\n"
		<< "Content-Type: text/html\r\n"
		<< "Content-Length: " << fileInfo.st_size << "\r\n" // Exact dynamic size!
		<< "\r\n";
		this->m_WriteBuffer = headerStream.str();
		
		//TODO Member 2: check if the output file containe headers
		this->m_ContentFileFd = open(this->m_FileContentPath.c_str(), O_RDONLY);
		if (this->m_ContentFileFd == -1)
		{
			ERROR_LOG("Could not open mock body file");
			return (500);
		}
		this->m_State = STATE_SENDING_BODY;
	}
	// 2. Subsequent loop iterations stream chunks smoothly
	else if (this->m_State == STATE_SENDING_BODY)
	{
		// Only read more from disk if our socket write buffer has cleared out.
		// This prevents loading a massive file into RAM all at once.
		if (this->m_WriteBuffer.empty())
		{
			int res = this->ReadFileContent();
			if (res == -1)
			{
				close(this->m_ContentFileFd);	
				return (500);
			}
			
			if (res == 0) // EOF reached and buffer is confirmed empty
			{
				close(this->m_ContentFileFd);
				this->m_State = STATE_RESPONSE_SENT;
			}
		}
	}
	return (200);
}

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
;