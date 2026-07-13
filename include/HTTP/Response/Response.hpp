#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <map>

class Request;
class LocationConfig; 
class Client;

class Response
{
    private:
        std::string _statusLine;
        std::string _headers;
        std::string _body;
        std::string _rawResponse;

        static std::map<std::string, std::string> _mimeTypes;
        
        void buildStatusLine(int statusCode);
        void initMimeTypes();
        std::string generateAutoindex(const std::string& dirPath, const std::string& uri);

        void handleGet(Client& client);
        void handleDelete(Client& client);
        void handlePost(Client& client);

    public:
        Response();
        Response(Client& client);
        ~Response();

        void generateErrorResponse(int statusCode);
        std::string getRawResponse() const;
};

#endif