/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 12:37:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/20 13:03:36 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/utils.hpp"

std::string	GenerateTmpFileName(std::string contex)
{
	char	buf[100];
	struct tm	tm = Timer::GetTime();
	
	std::strftime(buf, sizeof(buf), "_%s", &tm);
	return ("/tmp/" + contex + std::string(buf) + ".tmp");
}


std::string toString(int port)
{
    std::stringstream ss;
    ss << port;
    return ss.str();
}


static void PrintStringVector(const std::vector<std::string>& vec)
{
    std::cout << "[ ";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        std::cout << "\"" << vec[i] << "\"";
        if (i + 1 != vec.size())
            std::cout << ", ";
    }
    std::cout << " ]";
}


void PrintConfigTree(const ConfigTree& tree)
{
    std::cout << "=========================================\n";
    std::cout << "           CONFIG TREE\n";
    std::cout << "=========================================\n\n";

    for (size_t i = 0; i < tree.servers.size(); ++i)
    {
        const ServerConfig& srv = tree.servers[i];

        std::cout << "SERVER #" << i + 1 << "\n";
        std::cout << "-----------------------------------------\n";

        std::cout << "Server Name            : " << srv.server_name << "\n";

        std::cout << "Listen                 : ";
        for (size_t j = 0; j < srv.listens.size(); ++j)
        {
            std::cout << srv.listens[j].host << ":" << srv.listens[j].port;
            if (j + 1 != srv.listens.size())
                std::cout << ", ";
        }
        std::cout << "\n";

        std::cout << "Root                   : " << srv.root << "\n";
        std::cout << "Index                  : " << srv.index << "\n";
        std::cout << "Autoindex              : " << (srv.autoindex ? "on" : "off") << "\n";
        std::cout << "Client Max Body Size   : " << srv.client_max_body_size << "\n";

        std::cout << "\nRaw Directives:\n";
        for (std::map<std::string, std::vector<std::string> >::const_iterator it = srv.directives.begin();
             it != srv.directives.end(); ++it)
        {
            std::cout << "  " << it->first << " = ";
            PrintStringVector(it->second);
            std::cout << "\n";
        }

        std::cout << "\nLocations (" << srv.locations.size() << ")\n";

        for (size_t j = 0; j < srv.locations.size(); ++j)
        {
            const LocationConfig& loc = srv.locations[j];

            std::cout << "\n  LOCATION #" << j + 1 << "\n";
            std::cout << "  -----------------------------\n";
            std::cout << "  Path                  : " << loc.path << "\n";
            std::cout << "  Root                  : " << loc.root << "\n";
            std::cout << "  Index                 : " << loc.index << "\n";
            std::cout << "  Autoindex             : " << (loc.autoindex ? "on" : "off") << "\n";
            std::cout << "  Client Max Body Size  : " << loc.client_max_body_size << "\n";
            std::cout << "  upload_file           :" << loc.upload_file << "\n";

            std::cout << "  Allow Methods         : ";
            PrintStringVector(loc.allow_methods);
            std::cout << "\n";

            std::cout << "  CGI:\n";
            if (loc.cgi.empty())
                std::cout << "    (none)\n";
            else
            {
                for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin();
                     it != loc.cgi.end(); ++it)
                {
                    std::cout << "    " << it->first
                              << " -> " << it->second << "\n";
                }
            }

            std::cout << "  Return Directive      : ";
            if (loc.return_directive.first == 0)
                std::cout << "(none)\n";
            else
                std::cout << loc.return_directive.first
                          << " -> "
                          << loc.return_directive.second
                          << "\n";

            std::cout << "  Error Pages:\n";
            if (loc.error_pages.empty())
                std::cout << "    (none)\n";
            else
            {
                for (std::map<int, std::string>::const_iterator it = loc.error_pages.begin();
                     it != loc.error_pages.end(); ++it)
                {
                    std::cout << "    "
                              << it->first
                              << " -> "
                              << it->second
                              << "\n";
                }
            }

            std::cout << "  Raw Directives:\n";
            for (std::map<std::string, std::vector<std::string> >::const_iterator it = loc.directives.begin();
                 it != loc.directives.end(); ++it)
            {
                std::cout << "    " << it->first << " = ";
                PrintStringVector(it->second);
                std::cout << "\n";
            }
        }

        std::cout << "\n";
    }

    std::cout << "=========================================\n";
}

// void PrintParsedRequest(const Request& req) 
// {
//     std::cout << "\n\033[1;35m" << std::string(60, '=') << "\033[0m\n";
//     std::cout << "\033[1;33m[+] HTTP REQUEST PARSER OUTPUT [+]\033[0m\n";
//     std::cout << "\033[1;35m" << std::string(60, '-') << "\033[0m\n\n";

//     // 1. Resolve Method String
//     std::string methodStr = "UNKNOWN";
//     if (req.GetMethod() == HTTP_GET) methodStr = "GET";
//     else if (req.GetMethod() == HTTP_POST) methodStr = "POST";
//     else if (req.GetMethod() == HTTP_DELETE) methodStr = "DELETE";

//     // 2. Print Request Line Data
//     std::cout << "\033[1;32m[REQUEST LINE]\033[0m\n";
//     std::cout << "  Method  : \033[0;36m" << methodStr << "\033[0m\n";
//     std::cout << "  Path    : \033[0;36m" << req.GetPath() << "\033[0m\n";
//     std::cout << "  Query   : \033[0;36m" << (req.GetQuery().empty() ? "(none)" : req.GetQuery()) << "\033[0m\n";
//     std::cout << "  Version : \033[0;36m" << (req.GetVesrion().empty() ? "(none)" : req.GetVesrion()) << "\033[0m\n\n";

//     // 3. Print Headers Map
//     std::cout << "\033[1;32m[HEADERS]\033[0m\n";
//     const std::map<std::string, std::string>& headers = req.GetHeaders();
//     if (headers.empty()) {
//         std::cout << "  (none)\n";
//     } else {
//         std::map<std::string, std::string>::const_iterator it;
//         for (it = headers.begin(); it != headers.end(); ++it) {
//             // Notice how the keys will all be perfectly lowercase because of our Trim/ToLower logic!
//             std::cout << "  " << it->first << " : \033[0;36m" << it->second << "\033[0m\n";
//         }
//     }
//     std::cout << "\n";

//     // 4. Print Body Data
//     std::cout << "\033[1;32m[BODY]\033[0m\n";
//     std::cout << "  Expected Content-Length : " << req.GetContentLength() << " bytes\n";
//     std::cout << "  Actual Body Payload     :\n\033[0;36m" << (req.GetBody().empty() ? "  (empty)" : req.GetBody()) << "\033[0m\n\n";

//     // 5. Print State & Errors
//     std::cout << "\033[1;32m[INTERNAL STATE]\033[0m\n";
//     if (req.GetErrorCode() != 0) {
//         std::cout << "  Error Code : \033[1;31m" << req.GetErrorCode() << " (Parsing Failed!)\033[0m\n";
//     } else {
//         std::cout << "  Error Code : 0 (No Errors)\n";
//     }
    
//     std::cout << "  Is Ready?  : ";
//     if (req.GetState() == PARSE_COMPLETE) {
//         std::cout << "\033[1;32mYES (Ready for Router)\033[0m\n";
//     } else {
//         std::cout << "\033[1;33mNO (Waiting for more data from epoll...)\033[0m\n";
//     }

//     std::cout << "\033[1;35m" << std::string(60, '=') << "\033[0m\n\n";
// }
