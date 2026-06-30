#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include "../include/HTTP/Request/Request.hpp"
#include <string>

class RequestParser {
    private:
        // helper Utilities
        static std::string ToLowercase(const std::string& str);
        static std::string Trim(const std::string& str);
        
        static void ParseRequestLine(Request& req, const std::string& line);
        static void ParseHeader(Request& req, const std::string& line);

    public:
        // main func
        static bool Parse(Request& req, std::string& rawBuffer);
};

#endif