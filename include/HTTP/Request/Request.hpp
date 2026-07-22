/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wahmane <wahmane@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 23:06:16 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/20 16:28:07 by wahmane          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "Utils/utils.hpp"
#include "Core/HttpStatus.hpp"

// Core Enums
enum HttpMethod { HTTP_GET, HTTP_POST, HTTP_DELETE, HTTP_UNKNOWN };
enum ParseState { PARSE_REQUEST_LINE, PARSE_HEADERS, PARSE_BODY, PARSE_COMPLETE, PARSE_ERROR };

// A struct to hold the temp file paths and their real filenames
struct MultipartPart {
    std::string realFileName;
    std::string tmpFilePath;
    int         fd;
};

class Request {
	private:
		HttpMethod                          m_Method;
		std::string							m_MethodString;
		std::string                         m_Path;    
		std::string                         m_Query;   // for CGI in case or ?id=1
		std::string                         m_Version; //  HTTP version
		std::map<std::string, std::string>  m_Headers; // headers map
		std::map<std::string, std::string>	m_Cookies;
		
		std::string                         m_BodyFilePath;
		int                                 m_BodyFd;
		// should we change this size_t ?
		size_t                              m_BodyReceived;

		ParseState                          m_State;
		size_t                              m_ContentLength; // setting the content length
		HttpStatusCode						m_ErrorCode; // error code

		// Multipart file upload
		std::string							m_Boundary;
		bool        						m_IsMultipart;
		std::vector<MultipartPart>			m_Parts;

	public:
		Request();
		~Request();
		
		// Setters
		void	SetState(ParseState s);
		void	SetErrorCode(HttpStatusCode c);
		void	SetMethod(HttpMethod m);
		void	SetMethodString(std::string& m);

		void	SetPath(const std::string& p);
		void	SetQuery(const std::string& q);
		void	SetVersion(const std::string& v);
		void	AddHeader(const std::string& k, const std::string& v);
		void	AddCookie(const std::string& key, const std::string& value);
				// void	AppendBody(const std::string& d);
		void	SetContentLength(size_t l);

		// Getters
		ParseState			GetState() const;
		HttpStatusCode		GetErrorCode() const;
		HttpMethod  		GetMethod() const;
		const std::string&	GetMethodString() const;

		const std::string&	GetPath() const;
		const std::string&	GetQuery() const;
		// const std::string& GetBody() const;
		const std::string&	GetVesrion() const;
		const std::map<std::string, std::string>& GetHeaders() const;
		const std::map<std::string, std::string>& GetCookies() const;

		size_t              GetContentLength() const;
		std::string         GetHeader(const std::string& key) const;
		std::string			GetCookie(const std::string& key) const;
		const std::string&  GetBodyFilePath() const;
		// handling boddy 

		bool        OpenBodyFile();
		bool        AppendBody(const char* buffer, size_t len);
		void        CloseBodyFile();
		int         GetBodyFd() const;
		size_t      GetBodyReceived() const;

		// Multipart Setters & Getters
		void                SetIsMultipart(bool val);
		bool                IsMultipart() const;
		void                SetBoundary(const std::string& boundary);
		const std::string&  GetBoundary() const;
		void                AddBodyReceived(size_t len);

		// Multipart File Handling
		bool                HasOpenMultipartPart() const;
		bool                OpenNewMultipartPart(const std::string& filename);
		bool                WriteToCurrentMultipartPart(const std::string& data);
		void                CloseCurrentMultipartPart();
		const std::vector<MultipartPart>& GetParts() const;


};

#endif

