#pragma once
#include "ConfigParser.hpp"
#include <cstdlib>
#include <iostream>


// Represents one parsed "listen" directive.
struct ListenConfig
{
    std::string host;
    int         port;
};

// Represents one unique listening socket.
//
// Example:
//
// listen 8080;
//
// can have multiple virtual servers:
//
// listen 8080
//     ├── Server A
//     ├── Server B
//     └── Server C
//
// without this we can combiene multiple servers to one socket
struct ResolvedListen
{
    ListenConfig                        listen;

    // Every server block sharing this listen socket.
    std::vector<const ServerConfig*>    servers;
};

class ConfigResolver
{
private:

    const ConfigTree&                   m_Config;

    std::vector<ResolvedListen>         m_RuntimeListens;

private:
    // we parse the listen value 127.0.0.1:8080 or 8080 and give it the default 0.0.0.0
    ListenConfig ParseListenValue(const std::string& value) const;

    void BuildRuntimeListens();

    // Search an already-created runtime listen.
    ResolvedListen* FindRuntimeListen(const ListenConfig& listen);
    // with this we get the exact path for the URI
    bool IsPrefixMatch(const std::string& location,const std::string& uri) const;

public:

    ConfigResolver(const ConfigTree& config);
    ~ConfigResolver();

    // Used by Webserv::Init()
    const std::vector<ResolvedListen>& GetRuntimeListens() const;

    // Used later by the router.
    const ServerConfig* GetServerBy_Ip_Port_Host(const std::string& localIp,int port,const std::string& hostHeader) const;

    const LocationConfig* GetLocationBy_Server_Uri(const ServerConfig& server,const std::string& uri) const;
};
