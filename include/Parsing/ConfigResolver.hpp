#pragma once
#include "ConfigParser.hpp"
#include <cstdlib>

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

    ListenConfig ParseListenValue(
        const std::string& value) const;

    void BuildRuntimeListens();

    // Search an already-created runtime listen.
    ResolvedListen* FindRuntimeListen(
        const ListenConfig& listen);

public:

    ConfigResolver(const ConfigTree& config);
    ~ConfigResolver();

    // Used by Webserv::Init()
    const std::vector<ResolvedListen>&
    GetRuntimeListens() const;

    // Used later by the router.
    const ServerConfig* ResolveServer(
        int port,
        const std::string& host) const;

    const LocationConfig* ResolveLocation(
        const ServerConfig& server,
        const std::string& uri) const;
};
