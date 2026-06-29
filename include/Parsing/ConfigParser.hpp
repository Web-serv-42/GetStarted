#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

// =========================================================================
// DATA STRUCTURES (The "Abstract Syntax Tree")
// =========================================================================

struct LocationConfig {
    std::string                         path;
    std::map<std::string, std::string>  directives;
};

struct ServerConfig {
    std::map<std::string, std::string>  directives;
    std::vector<LocationConfig>         locations;
};

struct ConfigTree {
    std::vector<ServerConfig>           servers;
};

// =========================================================================
// THE PARSER CLASS
// =========================================================================

class ConfigParser {
    private:
        std::vector<std::string> m_Tokens;
        size_t                   m_Pos;

        // Helpers for iterating through tokens
        bool                IsEOF() const;
        const std::string&  CurrentToken() const;
        std::string         Consume();
        void                Expect(const std::string& expectedToken);

        // State Machine Functions
        void                ParseDirective(std::map<std::string, std::string>& directivesMap);
        LocationConfig      ParseLocationBlock();
        ServerConfig        ParseServerBlock();

    public:
        ConfigParser(const std::vector<std::string>& tokens);
        ~ConfigParser();

        // Entry point that returns the fully built tree
        ConfigTree          Parse();
};

#endif