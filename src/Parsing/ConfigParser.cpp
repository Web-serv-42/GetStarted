#include "../include/Parsing/ConfigParser.hpp"
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

bool ConfigParser::IsDirective(const std::string& token)
{
    return token == "listen"
        || token == "root"
        || token == "cgi"
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

    if (key == "server_name"
        || key == "allow_methods"
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

void ConfigParser::ParseDirective(std::map<std::string, std::vector<std::string> >& directivesMap)
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
    // to handle multiple CGI AND PORTS
    if (key == "listen" || key == "cgi")
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
        ParseDirective(loc.directives);
    }

    Expect("}");
    return loc;
}


// =========================================================================
// STATE 3: PARSE SERVER (e.g., "server { ... }")
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
            ParseDirective(server.directives);
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

    return tree;
}