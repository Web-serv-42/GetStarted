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

            // Does this socket already exist?
            ResolvedListen* runtime = FindRuntimeListen(cfg);

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


const ServerConfig* ConfigResolver::ResolveServer(
    int port,
    const std::string& host) const
{
    for (size_t i = 0; i < m_RuntimeListens.size(); ++i)
    {
        if (m_RuntimeListens[i].listen.port != port)
            continue;

        const std::vector<const ServerConfig*>& servers =
            m_RuntimeListens[i].servers;

        // First server is the default one.
        const ServerConfig* defaultServer = servers[0];

        for (size_t j = 0; j < servers.size(); ++j)
        {
            std::map<std::string,
                     std::vector<std::string> >::const_iterator it;

            it = servers[j]->directives.find("server_name");

            if (it == servers[j]->directives.end())
                continue;

            for (size_t k = 0; k < it->second.size(); ++k)
            {
                if (it->second[k] == host)
                    return servers[j];
            }
        }

        // No Host match.
        return defaultServer;
    }

    return NULL;
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

// std::vector<ListenConfig> ConfigResolver::GetListenConfigs() const
// {
//     std::vector<ListenConfig> result;
    
//     for (size_t i = 0; i < m_Config.servers.size(); ++i)
//     {
//         std::map<std::string,
//                  std::vector<std::string> >::const_iterator it;

//         it = m_Config.servers[i].directives.find("listen");
//         //  throw error if no location dirrective found
//         if (it == m_Config.servers[i].directives.end())
//         {
//             throw std::runtime_error(
//             "Configuration Error: server block is missing a 'listen' directive"
//             );
//         }

//         const std::vector<std::string>& values = it->second;

//         for (size_t j = 0; j < values.size(); ++j)
//             result.push_back(ParseListenValue(values[j]));
//     }

//     return result;
// }