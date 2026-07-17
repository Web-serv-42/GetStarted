// #ifndef RESPONSE_HPP
// #define RESPONSE_HPP
#pragma once 

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <map>
// #include "Client/Client.hpp"
#include "HTTP/Request/Request.hpp"
#include "Parsing/ConfigResolver.hpp"
#include "Core/HttpStatus.hpp"

class Request;
struct LocationConfig; 
class Client;

class Response
{
    private:
        Routing    m_Routing;
        Request    m_Request;
        std::string _statusLine;
        std::string _headers;
        std::string _body; // headers + body = small fullReponse usage: GET failure, POST succes and failure, DELETE success and failure [failure: samll, error page]
        std::string _filePath;
        std::string _rawResponse;

        static std::map<std::string, std::string> _mimeTypes;
        
        void buildStatusLine(HttpStatusCode statusCode);
        void initMimeTypes();
        std::string generateAutoindex(const std::string& dirPath, const std::string& uri);

        HttpStatusCode  handleGet();
        HttpStatusCode  handleDelete();
        HttpStatusCode  handlePost();

    public:
        Response();
        Response(Routing routing, Request request);
        ~Response();

        HttpStatusCode Run();

        void generateErrorResponse(HttpStatusCode statusCode);
        std::string getRawResponse() const;

        void handleError(HttpStatusCode statusCode);
        const std::string& getFilePath() const;
};

// #endif   