/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 23:06:16 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/03 22:38:20 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../include/HTTP/Request/Request.hpp"
#include <iostream>

Request::Request() : m_Method(HTTP_UNKNOWN), m_State(PARSE_REQUEST_LINE), m_ContentLength(0), m_ErrorCode(0) {}

// Setters
void Request::SetState(ParseState s) { m_State = s; }
void Request::SetErrorCode(int c) { m_ErrorCode = c; }
void Request::SetMethod(HttpMethod m) { m_Method = m; }
void Request::SetPath(const std::string& p) { m_Path = p; }
void Request::SetQuery(const std::string& q) { m_Query = q; }
void Request::SetVersion(const std::string& v) { m_Version = v; }
void Request::AddHeader(const std::string& k, const std::string& v) { m_Headers[k] = v; }
void Request::AppendBody(const std::string& d) { m_Body += d; }
void Request::SetContentLength(size_t l) { m_ContentLength = l; }

// Getters
ParseState  Request::GetState() const { return m_State; }

int         Request::GetErrorCode() const { return m_ErrorCode; }

HttpMethod  Request::GetMethod() const { return m_Method; }

const std::string& Request::GetPath() const { return m_Path; }

const std::string& Request::GetQuery() const { return m_Query; }

const std::string& Request::GetBody() const { return m_Body; }

const std::string& Request::GetVesrion() const { return m_Version;}

const std::map<std::string, std::string>& Request::GetHeaders() const {
    return m_Headers;
}

size_t      Request::GetContentLength() const { return m_ContentLength; }

std::string Request::GetHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = m_Headers.find(key);
    if (it != m_Headers.end()) {
        return it->second;
    }
    return ""; // return empty string if header doesnt exist , we could check it after 
}