#include "Parsing/ConfigResolver.hpp"
#include <stdexcept>


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

        if (server.listens.empty())
            throw std::runtime_error("Configuration Error: server block missing listen directive");

        for (size_t j = 0; j < server.listens.size(); ++j)
        {   
            ListenConfig cfg = server.listens[j];
            // cfg produce host:port 
            // Does this socket already exist?
            ResolvedListen* runtime = FindRuntimeListen(cfg);
            // if we find the same server with the same port meaning duplicated 
            // directive listen in in 2+ viruatl servers we return a  pointer runtime to that object
            // 
            if (runtime)
            {   
                // we reject the server_name and listen duplication 
                // Same socket -> add another virtual server.
                for (size_t k = 0; k < runtime->servers.size(); ++k)
                {
                    if (runtime->servers[k]->server_name == server.server_name)
                    {
                        throw std::runtime_error(
                            "Configuration Error: duplicate server_name \"" +
                            server.server_name +
                            "\" for listen " +  
                            cfg.host + ":" + toString(cfg.port));
                    }
                }
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
    if (location == "/")
        return true;

    // what we do here is that if the location ends with  '/' 
    // we remove it just for comparison with URI
    std::string normalized = location;

    if (normalized.size() > 1 &&
        normalized[normalized.size() - 1] == '/')
    {
        normalized.erase(normalized.size() - 1);
    }

    // URI shorter than the normalized location cannot match.
    if (uri.size() < normalized.size())
        return false;

    // URI must begin with the normalized location.
    if (uri.compare(0, normalized.size(), normalized) != 0)
        return false;

    // Exact match.
    if (uri.size() == normalized.size())
        return true;

    // after that we compaire the last characterand it should be '/'
    // example : downloadss this would match but we check the last character 
    return (uri[normalized.size()] == '/');
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
        if (servers[j]->server_name == hostHeader)
        {
            return servers[j];
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

// --- ROUTING RESOLVER for Responce

Routing ConfigResolver::ResolveRequest(
    const std::string& localIp,
    int port,
    const std::string& hostHeader,
    const std::string& uri) const
{
    Routing routing;

    routing.server = GetServerBy_Ip_Port_Host(localIp, port, hostHeader);

    if (routing.server == NULL)
        return routing;

    routing.location = GetLocationBy_Server_Uri(*routing.server, uri);

    if (routing.location == NULL)
        return routing;

    // Build the physical filesystem path.
    std::string remaining = uri;

    // 
    if (routing.location->path != "/")
        remaining.erase(0, routing.location->path.size());

    // we make sure that the URI is starting with a '/'
    if (!remaining.empty() && remaining[0] != '/')
        remaining.insert(0, "/");
    
    // this for case that the root => /var/www/ meanind ends with '/' we normalize that
    std::string root = routing.location->root;

    if (root.size() > 1 && root[root.size() - 1] == '/')
        root.erase(root.size() - 1);

    routing.filePath = root + remaining;

    // --- NEW: DETERMINE IF IT IS A CGI REQUEST ---
    if (routing.location != NULL)
    {
        // 1. Find the position of the last dot '.' in the path to get the extension
        size_t dotPos = routing.filePath.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            std::string extension = routing.filePath.substr(dotPos); // returns ".php" or ".py"

            // 2. Check if this specific extension exists in our location's CGI map
            std::map<std::string, std::string>::const_iterator it = routing.location->cgis.find(extension);
            if (it != routing.location->cgis.end())
            {
                // Extension found! Save the interpreter path to your routing object
                routing.cgiInterpreter = it->second; // "/usr/bin/php-cgi"
                routing.isCgi = true;
            }
        }
    }

    return routing;
}
