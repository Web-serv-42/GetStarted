#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <cstdlib>
// =========================================================================
// DATA STRUCTURES (The "Abstract Syntax Tree")
// =========================================================================
struct ListenConfig
{
    std::string host;
    int         port;
};

struct LocationConfig {
    // this part is handled from the parsing 
    std::string                         path;
    std::map<std::string, std::vector<std::string> > directives;
    // and from the parsing we get this infos
    // we add the importat dirrective and give the important one a defualt constructor
    // location inherit from server block 
    std::string                 root;
    std::string                 index;
    bool                        autoindex;
    size_t                      client_max_body_size;
    std::vector<std::string>    allow_methods;
    
    // CGI: map of extension to interpreter (e.g., {".py": "/usr/bin/python3"})
    std::map<std::string, std::string> cgi; 
    
    // Return: <status_code, url/text> (0 means no return directive)
    std::pair<int, std::string> return_directive; 
    
    // Error pages: map of status code to file path (e.g., {404: "/404.html"})
    std::map<int, std::string>  error_pages; 

    std::string upload_file;

    // Default constructor for safe fallbacks
    LocationConfig() : autoindex(false), client_max_body_size(1048576) {
        return_directive.first = 0;
    }
};

struct ServerConfig {
    // std::map<std::string, std::string>  directives;
    std::map<std::string, std::vector<std::string> > directives;
    std::vector<LocationConfig>         locations;
    // so now we enforce that this 2 are mandatory in the config file 

    std::vector<ListenConfig> listens;
    std::string  server_name; // forcing only one server_name per server

    // we give default since we dont have http{} and that would locaion inherit from
    std::string root;
    std::string index;
    std::map<int, std::string>  error_pages; 
    bool        autoindex;
    size_t      client_max_body_size;

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
        // void                ParseDirective(std::map<std::string, std::string>& directivesMap);
        void                ParseDirective(std::map<std::string, std::vector<std::string> >& directivesMap,std::map<int, std::string>& error_pages);
        LocationConfig      ParseLocationBlock();
        ServerConfig        ParseServerBlock();
        bool                IsDirective(const std::string& token);
        void                ValidateDirective(
                                const std::string& key,
                                const std::vector<std::string>& values,
                                const std::map<std::string,
                                std::vector<std::string> >& directivesMap);
        // -- new for ez routing
        ListenConfig ParseListenValue(const std::string& value) const;
        void          FinalizeAndInherit(ConfigTree& tree);

    public:
        ConfigParser(const std::vector<std::string>& tokens);
        ~ConfigParser();
        // Entry point that returns the fully built tree
        ConfigTree          Parse();
};

#endif