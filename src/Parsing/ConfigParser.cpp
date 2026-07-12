#include "Parsing/ConfigParser.hpp"
#include "Utils/utils.hpp"
#include <iostream>
// =========================================================================
// CONSTRUCTOR & DESTRUCTOR
// =========================================================================
ConfigParser::ConfigParser(const std::vector<std::string>& tokens) : m_Tokens(tokens), m_Pos(0) {}
ConfigParser::~ConfigParser() {}

// =========================================================================
// HELPER METHODS
// =========================================================================
bool ConfigParser::IsEOF() const 
{
    return m_Pos >= m_Tokens.size();
}

const std::string& ConfigParser::CurrentToken() const 
{
    if (IsEOF()) {
        throw std::runtime_error("Syntax Error: Unexpected End of File");
    }
    return m_Tokens[m_Pos];
}

std::string ConfigParser::Consume() 
{

    if (IsEOF()) {
        throw std::runtime_error("Syntax Error: Unexpected End of File");
    }
    return m_Tokens[m_Pos++];
}

void ConfigParser::Expect(const std::string& expectedToken) 
{
    if (IsEOF() || m_Tokens[m_Pos] != expectedToken) {
        std::string got = IsEOF() ? "EOF" : m_Tokens[m_Pos];
        // std::cout << "Syntax Error: Expected '" + expectedToken + "' but got '" + got + "'" << std::endl; 
        throw std::runtime_error("Syntax Error: Expected '" + expectedToken + "' but got '" + got + "'");
    }
    m_Pos++;
}


ListenConfig ConfigParser::ParseListenValue(const std::string& value) const
{
    ListenConfig listen_line;
    size_t colon = value.find(':');

    if (colon == std::string::npos) {
        listen_line.host = "0.0.0.0";
        listen_line.port = std::atoi(value.c_str());
    } else {
        listen_line.host = value.substr(0, colon);
        listen_line.port = std::atoi(value.substr(colon + 1).c_str());
    }

    if (listen_line.port < 1 || listen_line.port > 65535) {
        throw std::runtime_error("Invalid listen port: " + value);
    }

    return listen_line;
}


bool ConfigParser::IsDirective(const std::string& token)
{
    return token == "listen"
        || token == "root"
        || token == "cgi"
        || token == "upload_file"
        || token == "index"
        || token == "return"
        || token == "autoindex"
        || token == "server_name"
        || token == "error_page"
        || token == "client_max_body_size"
        || token == "allow_methods";
}

// =========================================================================
// STATE 1: PARSE DIRECTIVE (e.g., "listen 8080;" or "root /var/www;")
// =========================================================================


void ConfigParser::ValidateDirective(
    const std::string& key,
    const std::vector<std::string>& values,
    const std::map<std::string,
                   std::vector<std::string> >& directivesMap)
{
    // ---------- Directives expecting exactly one argument ----------

    if (key == "listen"
        || key == "root"
        || key == "index"
        || key == "autoindex"
        || key == "server_name"
        || key == "upload_file"
        || key == "client_max_body_size")
    {
        if (values.size() != 1)
        {
            throw std::runtime_error(
                "Syntax Error: '" + key +
                "' expects exactly one argument"
            );
        }
    }

    // ---------- Directives expecting one or more arguments ----------

    if (key == "allow_methods"
        || key == "error_page")
    {
        if (values.empty())
        {
            throw std::runtime_error(
                "Syntax Error: '" + key +
                "' expects at least one argument"
            );
        }
    }

    // ---------- autoindex ----------

    if (key == "autoindex")
    {
        if (values[0] != "on" && values[0] != "off")
        {
            throw std::runtime_error(
                "Configuration Error: 'autoindex' must be 'on' or 'off'"
            );
        }
    }

    // ---------- return ----------

    if (key == "return")
    {
        if (values.size() < 1 || values.size() > 2)
        {
            throw std::runtime_error(
                "Syntax Error: 'return' expects one or two arguments"
            );
        }

        for (size_t i = 0; i < values[0].size(); ++i)
        {
            if (!std::isdigit(values[0][i]))
            {
                throw std::runtime_error(
                    "Configuration Error: invalid return status code '"
                    + values[0] + "'"
                );
            }
        }
    }

    // ---------- Duplicate directives ----------

    if (key == "root"
        || key == "index"
        || key == "server_name"
        || key == "listen"
        || key == "autoindex"
        || key == "client_max_body_size"
        || key == "return")
    {
        if (directivesMap.find(key) != directivesMap.end())
        {
            throw std::runtime_error(
                "Configuration Error: duplicate '" +
                key + "' directive"
            );
        }
    }
    // ---------- CGI ----------

    if (key == "cgi")
    {
        if (values.size() != 2)
        {
            throw std::runtime_error(
                "Syntax Error: 'cgi' expects exactly two arguments"
            );
        }
        // extension + interpreter 
        if (values[0].empty() || values[0][0] != '.')
        {
            throw std::runtime_error(
                "Configuration Error: CGI extension must begin with '.'"
            );
        }
    }
}

void ConfigParser::ParseDirective(std::map<std::string, std::vector<std::string> >& directivesMap,std::map<int, std::string>& error_pages)
{
    std::string key;
    std::vector<std::string> values;

    key = Consume();

    if (!IsDirective(key))
    {
        throw std::runtime_error(
            "Configuration Error: Unknown directive '" + key + "'"
        );
    }

    while (!IsEOF() && CurrentToken() != ";")
    {
        if (IsDirective(CurrentToken()))
        {
            throw std::runtime_error(
                "Syntax Error: Expected ';' after '" + key + "'"
            );
        }

        values.push_back(Consume());
    }

    Expect(";");

    ValidateDirective(key, values, directivesMap);

    if (key == "error_page")
    {
        if (values.size() >= 2)
        {
            const std::string& path = values.back();

            for (size_t i = 0; i + 1 < values.size(); ++i)
            {
                int status = std::atoi(values[i].c_str());
                error_pages[status] = path;
            }
        }
    }

    // to handle multiple CGI AND PORTS
    if (key == "cgi")
    {
        directivesMap[key].insert(
            directivesMap[key].end(),
            values.begin(),
            values.end());
    }
    else    
    {
        directivesMap[key] = values;
    }
}



// =========================================================================
// STATE 2: PARSE LOCATION (e.g., "location / { ... }")
// =========================================================================


LocationConfig ConfigParser::ParseLocationBlock() 
{
    LocationConfig loc;
    
    Expect("location");
    loc.path = Consume(); // Get the URI path (e.g., "/images")
    Expect("{");

    // Read everything inside the braces
    while (!IsEOF() && CurrentToken() != "}") 
    {
        ParseDirective(loc.directives,loc.error_pages);
    }

    Expect("}");
    return loc;
}


// =========================================================================
// STATE 3: PARSE SERVER ("server {}")
// =========================================================================


ServerConfig ConfigParser::ParseServerBlock() 
{
    ServerConfig server;

    Expect("server");
    Expect("{");

    while (!IsEOF() && CurrentToken() != "}") 
    {
        if (CurrentToken() == "location") {
            // Branch into Location State
            server.locations.push_back(ParseLocationBlock());
        } else {
            // Treat as a standard server-level directive
            ParseDirective(server.directives,server.error_pages);
        }
    }

    Expect("}");
    return server;
}


// =========================================================================
// MAIN ENTRY POINT
// =========================================================================


ConfigTree ConfigParser::Parse() 
{
    ConfigTree tree;
    
    // The root of the file should only contain 'server' blocks
    while (!IsEOF()) 
    {
        if (CurrentToken() == "server") {
            tree.servers.push_back(ParseServerBlock());
        } else {
            throw std::runtime_error("Syntax Error: Expected 'server' block at top level, got '" + CurrentToken() + "'");
        }
    }

    if (tree.servers.empty()) {
        throw std::runtime_error("Configuration Error: No valid server blocks found.");
    }

    FinalizeAndInherit(tree);

    // PrintConfigTree(tree); 

    return tree;
}

// =========================================================================
// PARSING FINALIZER  
// =========================================================================

void ConfigParser::FinalizeAndInherit(ConfigTree& tree) 
{
    for (size_t i = 0; i < tree.servers.size(); ++i) 
    {
        ServerConfig& srv = tree.servers[i];

        // 1. EXTRACT SERVER-LEVEL DEFAULTS
        srv.root = srv.directives.count("root") ? srv.directives["root"][0] : "/var/www/html";
        srv.index = srv.directives.count("index") ? srv.directives["index"][0] : "index.html";
        srv.autoindex = srv.directives.count("autoindex") && srv.directives["autoindex"][0] == "on";
        
        if (srv.directives.count("client_max_body_size"))
            srv.client_max_body_size = std::atoi(srv.directives["client_max_body_size"][0].c_str());
        else
            srv.client_max_body_size = 1048576; // 1MB default

        // Extract Single Server Name
        if (srv.directives.count("server_name") && !srv.directives["server_name"].empty()) {
            srv.server_name = srv.directives["server_name"][0]; // Only take the first one
        } else {
            srv.server_name = ""; // Empty string acts as the default catch-all
        }



        // Extract Listens
        if (srv.directives.count("listen")) {
            for (size_t l = 0; l < srv.directives["listen"].size(); ++l) {
                srv.listens.push_back(ParseListenValue(srv.directives["listen"][l]));
            }
        }

        // 2. INHERIT TO LOCATIONS
        for (size_t j = 0; j < srv.locations.size(); ++j) 
        {
            LocationConfig& loc = srv.locations[j];

            // here we handle the location duplication before inheritance
            for (size_t j = 0; j < srv.locations.size(); ++j)
            {
                for (size_t k = j + 1; k < srv.locations.size(); ++k)
                {
                    if (srv.locations[j].path == srv.locations[k].path)
                        throw std::runtime_error(
                            "Configuration Error: duplicate location \"" +
                            srv.locations[j].path + "\"");
                }
            }
            
            // Inherit standard directives
            loc.root = loc.directives.count("root") ? loc.directives["root"][0] : srv.root;
            loc.index = loc.directives.count("index") ? loc.directives["index"][0] : srv.index;
            loc.autoindex = loc.directives.count("autoindex") ? (loc.directives["autoindex"][0] == "on") : srv.autoindex;
            loc.client_max_body_size = loc.directives.count("client_max_body_size") ? std::atoi(loc.directives["client_max_body_size"][0].c_str()) : srv.client_max_body_size;
            
             loc.upload_file = loc.directives.count("upload_file") ? loc.directives["upload_file"][0] : "";
            // Allow Methods (Will automatically contain ["GET", "POST", "DELETE"])
            if (loc.directives.count("allow_methods")) {
                loc.allow_methods = loc.directives["allow_methods"];
            }
            
            // here we handle the error_pages 
            // we only inherite if we dont have a error_pages and the server block is also non-empty
            if (loc.error_pages.empty() && !srv.error_pages.empty())
            {
                loc.error_pages = srv.error_pages;
            }

            // in dirrective multiple cgi data is stored this way : [".py", "/usr/bin/python3", ".php", "/usr/bin/php-cgi"]
            // meaning we need to skip k += 2 ; to get Extension | interpreter 
            if (loc.directives.count("cgi")) {
                const std::vector<std::string>& cgi_vals = loc.directives["cgi"];
                for (size_t k = 0; k + 1 < cgi_vals.size(); k += 2) {
                    loc.cgi[cgi_vals[k]] = cgi_vals[k + 1];
                }
            }

            // Parse Return Directive (Status Code + URL/Text)
            if (loc.directives.count("return")) {
                const std::vector<std::string>& ret_vals = loc.directives["return"];
                loc.return_directive.first = std::atoi(ret_vals[0].c_str());
                if (ret_vals.size() > 1) {
                    loc.return_directive.second = ret_vals[1];
                }   
            }
        }
    }
}