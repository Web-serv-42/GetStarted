#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include "HTTP/Request/Request.hpp"
#include "Core/HttpStatus.hpp"
#include "Utils/utils.hpp"

#include <string>
#include <sstream>
#include <cstdlib>
#include <cctype>

class RequestParser {
    private:
        // helper Utilities
        static std::string ToLowercase(const std::string& str);
        static std::string Trim(const std::string& str);
        
        static void ParseRequestLine(Request& req, const std::string& line);
        static void ParseHeader(Request& req, const std::string& line);
        static void ParseCookies(Request& req, const std::string& line);

        static std::string  ExtractExtensionFromHeaders(const std::string& headers);
        static bool         ParseMultipartBody(Request& req, std::string& rawBuffer);


    public:
        // main func
        static bool Parse(Request& req, std::string& rawBuffer);
};

#endif