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
        throw std::runtime_error("Syntax Error: Expected '" + expectedToken + "' but got '" + got + "'");
    }
    m_Pos++;
}

// =========================================================================
// STATE 1: PARSE DIRECTIVE (e.g., "listen 8080;" or "root /var/www;")
// =========================================================================
void ConfigParser::ParseDirective(std::map<std::string, std::string>& directivesMap) 
{
    std::string key = Consume(); 
    std::string value = "";

    // Read all tokens as the value until we hit the semicolon
    while (!IsEOF() && CurrentToken() != ";") 
    {
        if (!value.empty()) value += " ";
        value += Consume();
    }
    Expect(";"); 

    // --- SECURITY GUARD: Catch Duplicate Singletons ---
    if (key == "root" || key == "listen" || key == "client_max_body_size") 
    {
        if (directivesMap.find(key) != directivesMap.end()) {
            throw std::runtime_error("Configuration Error: [emerg] '" + key + "' directive is duplicate");
        }
    }

    // --- LOGIC GUARD: Accumulate Plurals (e.g., multiple error_pages) ---
    if (key == "error_page" || key == "server_name" || key == "allow_methods") 
    {
        if (directivesMap.find(key) != directivesMap.end()) {
            directivesMap[key] += " " + value; // Append with a space
        } else {
            directivesMap[key] = value;
        }
        return;
    }

    // Standard insertion
    directivesMap[key] = value;
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