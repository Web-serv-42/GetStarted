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
