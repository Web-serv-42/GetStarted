/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 23:06:16 by abnsila           #+#    #+#             */
/*   Updated: 2026/06/30 15:21:58 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

// Core Enums
enum HttpMethod { HTTP_GET, HTTP_POST, HTTP_DELETE, HTTP_UNKNOWN };
enum ParseState { PARSE_REQUEST_LINE, PARSE_HEADERS, PARSE_BODY, PARSE_COMPLETE, PARSE_ERROR };

class Request {
    private:
        HttpMethod                          m_Method;
        std::string                         m_Path;    
        std::string                         m_Query;   // for CGI in case or ?id=1
        std::string                         m_Version; //  HTTP version
        std::map<std::string, std::string>  m_Headers; // headers map
        std::string                         m_Body; // we can change this after and stop it into a file
        
        ParseState                          m_State;
        size_t                              m_ContentLength; // setting the content length
        int                                 m_ErrorCode; // error code

    public:
        Request();
        
        // Setters
        void SetState(ParseState s);
        void SetErrorCode(int c);
        void SetMethod(HttpMethod m);
        void SetPath(const std::string& p);
        void SetQuery(const std::string& q);
        void SetVersion(const std::string& v);
        void AddHeader(const std::string& k, const std::string& v);
        void AppendBody(const std::string& d);
        void SetContentLength(size_t l);

        // Getters
        ParseState  GetState() const;
        int         GetErrorCode() const;
        HttpMethod  GetMethod() const;
        
        const std::string& GetPath() const;
        const std::string& GetQuery() const;
        const std::string& GetBody() const;
        const std::string& GetVesrion() const;
        const std::map<std::string, std::string>& GetHeaders() const;
        
        size_t      GetContentLength() const;
        std::string GetHeader(const std::string& key) const;
};

#endif

