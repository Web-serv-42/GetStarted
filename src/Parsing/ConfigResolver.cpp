#include "../../include/Parsing/ConfigResolver.hpp"


// ConfigResolver::ConfigResolver(const ConfigTree& config): m_Config(config) {}

ConfigResolver::ConfigResolver(const ConfigTree& config): m_Config(config)
{
    BuildRuntimeListens();
}

ConfigResolver::~ConfigResolver(){}

const std::vector<ResolvedListen>& ConfigResolver::GetRuntimeListens() const
{
    return m_RuntimeListens;
}

ResolvedListen* ConfigResolver::FindRuntimeListen(const ListenConfig& listen)
{
    for (size_t i = 0; i < m_RuntimeListens.size(); ++i)
    {
        if (m_RuntimeListens[i].listen.host == listen.host &&
            m_RuntimeListens[i].listen.port == listen.port)
        {
            return &m_RuntimeListens[i];
        }
    }

    return NULL;
}

void ConfigResolver::BuildRuntimeListens()
{
    for (size_t i = 0; i < m_Config.servers.size(); ++i)
    {
        const ServerConfig& server = m_Config.servers[i];

        std::map<std::string,
                 std::vector<std::string> >::const_iterator it;

        it = server.directives.find("listen");

        if (it == server.directives.end())
        {
            throw std::runtime_error(
                "Configuration Error: server block missing listen directive");
        }

        const std::vector<std::string>& listens = it->second;

        for (size_t j = 0; j < listens.size(); ++j)
        {
            ListenConfig cfg = ParseListenValue(listens[j]);
            // cfg produce host:port 
            // Does this socket already exist?
            ResolvedListen* runtime = FindRuntimeListen(cfg);
            // if we find the same server with the same port meaning duplicated 
            // directive listen in in 2+ viruatl servers we return a  pointer runtime to that object
            // 
            if (runtime)
            {
                // Same socket -> add another virtual server.
                runtime->servers.push_back(&server);
            }
            else
            {
                // New listening socket.
                ResolvedListen newListen;

                newListen.listen = cfg;

                newListen.servers.push_back(&server);

                m_RuntimeListens.push_back(newListen);
            }
        }
    }
}

bool ConfigResolver::IsPrefixMatch(
    const std::string& location,
    const std::string& uri) const
{
    // URI shorter than the location cannot match.
    if (uri.size() < location.size())
        return false;

    // The URI must begin with the location.
    if (uri.compare(0, location.size(), location) != 0)
        return false;

    // "/" matches everything
    if (location == "/")
        return true;

    // Exact match.
    if (uri.size() == location.size())
        return true;

    // The next character must be a path separator.
    return (uri[location.size()] == '/');
}


const ServerConfig* ConfigResolver::GetServerBy_Ip_Port_Host(
    const std::string& localIp, 
    int port, 
    const std::string& hostHeader) const
{
    const ResolvedListen* matchedListen = NULL;

    // WE TRY TO FIND THE SOCKET IP + PORT we are listening for with the config file first 
    for (size_t i = 0; i < m_RuntimeListens.size(); ++i)
    {
        if (m_RuntimeListens[i].listen.port == port && 
            m_RuntimeListens[i].listen.host == localIp)
        {
            matchedListen = &m_RuntimeListens[i];
            break; 
        }
    }

    // IF WE dont find any thing with the IP + PORT we compaire with the socket 
    // we have wildcard  (listen 8080;) in the config meanign we compairre 0.0.0.0 + PORT
    if (matchedListen == NULL)
    {
        for (size_t i = 0; i < m_RuntimeListens.size(); ++i)
        {
            if (m_RuntimeListens[i].listen.port == port && 
                m_RuntimeListens[i].listen.host == "0.0.0.0")
            {
                matchedListen = &m_RuntimeListens[i];
                break; 
            }
        }
    }

    if (matchedListen == NULL)
        return NULL;

    const std::vector<const ServerConfig*>& servers = matchedListen->servers;
    const ServerConfig* defaultServer = servers[0];
    
    // match the HTTP Host header "Example => Host: site1" against server_name configurations 
    for (size_t j = 0; j < servers.size(); ++j)
    {
        std::map<std::string, std::vector<std::string> >::const_iterator it;
        it = servers[j]->directives.find("server_name");
        if (it == servers[j]->directives.end())
            continue;

        for (size_t k = 0; k < it->second.size(); ++k)
        {
            if (it->second[k] == hostHeader)
            {
                return servers[j];
            }
        }
    }
    return defaultServer;
}



const LocationConfig* ConfigResolver::GetLocationBy_Server_Uri(
    const ServerConfig& server,
    const std::string& uri) const
{
    const LocationConfig* best = NULL;
    size_t longestMatch = 0;

    for (size_t i = 0; i < server.locations.size(); ++i)
    {
        const LocationConfig& location = server.locations[i];

        if (!IsPrefixMatch(location.path, uri))
            continue;

        if (location.path.size() > longestMatch)
        {
            longestMatch = location.path.size();
            best = &location;
        }
    }

    return best;
}

// --Listen Directive Config Resolver

ListenConfig ConfigResolver::ParseListenValue(const std::string& value) const
{
    ListenConfig listen_line;

    size_t colon = value.find(':');

    if (colon == std::string::npos)
    {
        listen_line.host = "0.0.0.0";
        listen_line.port = std::atoi(value.c_str());
    }
    else
    {
        listen_line.host = value.substr(0, colon);
        listen_line.port = std::atoi(value.substr(colon + 1).c_str());
    }

    if (listen_line.port < 1 || listen_line.port > 65535)
    {
        throw std::runtime_error(
            "Invalid listen port: " + value
        );
    }

    return listen_line;
}
