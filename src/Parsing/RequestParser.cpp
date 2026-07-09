#include "../../include/Parsing/RequestParser.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>


// =========================================================================
// HELPER METHODS
// =========================================================================
std::string RequestParser::ToLowercase(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

// trim the request cuz i could be inforced
std::string RequestParser::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t"); // we dont \t cuz there is no tabs between but u never know
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}
    
// =========================================================================
// PARSERS
// =========================================================================
void RequestParser::ParseRequestLine(Request& req, const std::string& line) {
    std::istringstream iss(line);
    std::string methodStr, rawUri, version;
    iss >> methodStr >> rawUri >> version;

    // 1. Map Method
    if (methodStr == "GET") req.SetMethod(HTTP_GET);
    else if (methodStr == "POST") req.SetMethod(HTTP_POST);
    else if (methodStr == "DELETE") req.SetMethod(HTTP_DELETE);
    else {
        req.SetErrorCode(400); // Bad Request
        req.SetState(PARSE_ERROR);
        return;
    }

    // 2. ignore what is comming after # edge case (Fragment identifier)
    size_t hashPos = rawUri.find('#');
    if (hashPos != std::string::npos) rawUri = rawUri.substr(0, hashPos);

    // 3. Split Path and Query String
    size_t questionPos = rawUri.find('?');
    if (questionPos != std::string::npos) {
        req.SetPath(rawUri.substr(0, questionPos));
        req.SetQuery(rawUri.substr(questionPos + 1));
    } else {
        req.SetPath(rawUri);
        req.SetQuery("");
    }
    
    req.SetVersion(version);
}

void RequestParser::ParseHeader(Request& req, const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        req.SetErrorCode(400);
        req.SetState(PARSE_ERROR);
        return;
    }
    // KeyName:[colon][space]Value\r\n | [KeyName: ]
    std::string key = ToLowercase(Trim(line.substr(0, colonPos)));
    std::string value = Trim(line.substr(colonPos + 1));
    req.AddHeader(key, value);
}

// =========================================================================
// MAIN PARSER LOOP
// =========================================================================
bool RequestParser::Parse(Request& req, std::string& rawBuffer) {
    while (req.GetState() != PARSE_COMPLETE && req.GetState() != PARSE_ERROR) {
        
        // --- PARSE REQUEST LINE ---
        if (req.GetState() == PARSE_REQUEST_LINE) {
            size_t pos = rawBuffer.find("\r\n");
            if (pos == std::string::npos) return false;

            ParseRequestLine(req, rawBuffer.substr(0, pos));
            rawBuffer.erase(0, pos + 2);
            
            if (req.GetState() != PARSE_ERROR) req.SetState(PARSE_HEADERS);
        }
        
        // --- PARSE HEADERS ---
        else if (req.GetState() == PARSE_HEADERS) {
            size_t pos = rawBuffer.find("\r\n");
            if (pos == std::string::npos) return false;

            // do we need to delete ? yes so the next time we dont read the same line again
            std::string line = rawBuffer.substr(0, pos);
            rawBuffer.erase(0, pos + 2);

            // need more work on this 
            // do we just get what http 1.0 needed or ust parse every thing ??
            if (line.empty()) { // End of headers
                std::string cl = req.GetHeader("content-length");
                if (!cl.empty()) {
                    req.SetContentLength(std::atoi(cl.c_str()));
                    req.SetState(PARSE_BODY);
                } else {
                    req.SetState(PARSE_COMPLETE);
                }
                // we need this since we are not working with http 1.1 we 
                // reject the "transfer-encoding" we use content-length
                if (req.GetHeader("transfer-encoding") == "chunked") {
                    req.SetErrorCode(501);
                    req.SetState(PARSE_ERROR);
                }
            } else {
                ParseHeader(req, line);
            }
        }
        
        // --- PARSE BODY ---
        else if (req.GetState() == PARSE_BODY) {
            size_t expected = req.GetContentLength();
            if (rawBuffer.length() >= expected) {
                req.AppendBody(rawBuffer.substr(0, expected));
                rawBuffer.erase(0, expected);
                req.SetState(PARSE_COMPLETE);
            } else {
                return false; // waiting for more data from epoll
            }
        }
    }
    return (req.GetState() == PARSE_COMPLETE); // return true parsing is done
}