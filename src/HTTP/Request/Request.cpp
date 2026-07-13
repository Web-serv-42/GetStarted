/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 23:06:16 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/13 16:03:14 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "HTTP/Request/Request.hpp"
#include "Core/HttpStatus.hpp"
#include <iostream>


Request::Request() : m_Method(HTTP_UNKNOWN), m_BodyFd(-1) , m_BodyReceived(0), m_State(PARSE_REQUEST_LINE),  m_ContentLength(0), m_ErrorCode(NORMAL) {}
Request::~Request(){}


// Setters
void	Request::SetState(ParseState s) { m_State = s; }
void	Request::SetErrorCode(HttpStatusCode c) { m_ErrorCode = c; }
void	Request::SetMethod(HttpMethod m) { m_Method = m; }
void	Request::SetMethodString(std::string& m) { m_MethodString = m; }
void	Request::SetPath(const std::string& p) { m_Path = p; }
void	Request::SetQuery(const std::string& q) { m_Query = q; }
void	Request::SetVersion(const std::string& v) { m_Version = v; }
void	Request::AddHeader(const std::string& k, const std::string& v) { m_Headers[k] = v; }
void	Request::AddCookie(const std::string& k, const std::string& v) { m_Cookies[k] = v; }
// void	Request::AppendBody(const std::string& d) { m_Body += d; }
void	Request::SetContentLength(size_t l) { m_ContentLength = l; }

// Getters
ParseState  Request::GetState() const { return m_State; }

HttpStatusCode	Request::GetErrorCode() const { return m_ErrorCode; }

HttpMethod  Request::GetMethod() const { return m_Method; }

const std::string&  Request::GetMethodString() const { return m_MethodString; }

const std::string& Request::GetPath() const { return m_Path; }

const std::string& Request::GetQuery() const { return m_Query; }

// const std::string& Request::GetBody() const { return m_Body; }

const std::string& Request::GetVesrion() const { return m_Version;}

const std::map<std::string, std::string>& Request::GetHeaders() const { return m_Headers; }

const std::map<std::string, std::string>& Request::GetCookies() const { return m_Cookies; }

size_t      Request::GetContentLength() const { return m_ContentLength; }

std::string Request::GetHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = m_Headers.find(key);
	if (it != m_Headers.end()) {
		return it->second;
	}
	return ""; // return empty string if header doesnt exist , we could check it after 
}

std::string	Request::GetCookie(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = this->m_Cookies.find(key);
    if (it != this->m_Cookies.end())
        return it->second;
    return "";
}

// (Forbidden)  body appending to file

// bool Request::OpenBodyFile()
// {
//     if (m_BodyFd != -1)
//         return true;

//     char path[] = "./tmp/Request_body_XXXXXX";

//     m_BodyFd = mkstemp(path);

//     // std::cout << "CReated file FD [ " << m_BodyFd << " ] , name => " << path << std::endl;

//     if (m_BodyFd == -1)
//         return false;

//     // unlink it will delete the file name if no process have the file open
//     // unlink(path);

//     return true;
// }

bool Request::OpenBodyFile()
{
	// If it's already open, just keep using it (epoll chunking)
	if (m_BodyFd != -1)
		return true;

	// Use your custom function instead of mkstemp
	m_BodyFilePath = GenerateTmpFileName("request_body");

	// Open using allowed syscall (read/write, create if missing, truncate if exists)
	m_BodyFd = open(m_BodyFilePath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);

	return (m_BodyFd != -1);
}

bool Request::AppendBody(const char* buffer, size_t len)
{
	if (m_BodyFd == -1)
		return false;

	ssize_t written = write(m_BodyFd, buffer, len);
	// what if written return -1 ? 
	// shoulde we return fasle ?
	if (written != (ssize_t)len)
		return false;

	m_BodyReceived += written;

	return true;
}

void Request::CloseBodyFile()
{
	if (m_BodyFd != -1)
	{
		close(m_BodyFd);
		m_BodyFd = -1;
	}
}

const std::string&  Request::GetBodyFilePath() const
{
	return m_BodyFilePath;
}

int Request::GetBodyFd() const
{
	return m_BodyFd;
}

size_t  Request::GetBodyReceived() const
{
	return m_BodyReceived;
}
