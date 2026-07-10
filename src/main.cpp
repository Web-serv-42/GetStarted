/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/02 15:50:46 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../include/Parsing/Lexer.hpp"
#include "../include/HTTP/Request/Request.hpp"
#include "../include/Parsing/ConfigParser.hpp"
#include "../include/Parsing/RequestParser.hpp"

// Helper function to read the file
std::string ReadFileToString(const char* filepath) 
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Helper function to visualize the parsed Abstract Syntax Tree (AST)
void PrintParsedConfig(const ConfigTree& tree)
{
    std::cout << "\n\033[1;35m"
              << std::string(60, '=')
              << "\033[0m\n";

    std::cout << "\033[1;33m[+] PARSER OUTPUT ("
              << tree.servers.size()
              << " Servers Found) [+]\033[0m\n";

    std::cout << "\033[1;35m"
              << std::string(60, '-')
              << "\033[0m\n\n";

    for (size_t i = 0; i < tree.servers.size(); ++i)
    {
        std::cout << "\033[1;32m=== SERVER "
                  << (i + 1)
                  << " ===\033[0m\n";

        // ================= Server Directives =================
        std::map<std::string, std::vector<std::string> >::const_iterator sIt;

        for (sIt = tree.servers[i].directives.begin();
             sIt != tree.servers[i].directives.end();
             ++sIt)
        {
            std::cout << "  "
                      << sIt->first
                      << " : \033[0;36m";

            for (size_t j = 0; j < sIt->second.size(); ++j)
            {
                std::cout << sIt->second[j];

                if (j + 1 != sIt->second.size())
                    std::cout << " ";
            }

            std::cout << "\033[0m\n";
        }

        // ================= Location Blocks =================
        for (size_t j = 0; j < tree.servers[i].locations.size(); ++j)
        {
            const LocationConfig& loc = tree.servers[i].locations[j];

            std::cout << "\n  \033[1;34m--- Location: "
                      << loc.path
                      << " ---\033[0m\n";

            std::map<std::string,
                     std::vector<std::string> >::const_iterator lIt;

            for (lIt = loc.directives.begin();
                 lIt != loc.directives.end();
                 ++lIt)
            {
                std::cout << "      "
                          << lIt->first
                          << " : \033[0;36m";

                for (size_t k = 0; k < lIt->second.size(); ++k)
                {
                    std::cout << lIt->second[k];

                    if (k + 1 != lIt->second.size())
                        std::cout << " ";
                }

                std::cout << "\033[0m\n";
            }
        }

        std::cout << "\n";
    }

    std::cout << "\033[1;35m"
              << std::string(60, '=')
              << "\033[0m\n\n";
}

#include "Server/Webserv.hpp"


int main(int argc, char const *argv[])
{
	// (void)argc;
	// (void)argv;
    if (argc != 2) {
        std::cerr << "Usage: ./webserve <path_to_config_file>" << std::endl;
        return 1;
    }

	Webserv	engine;

    std::string configContent = ReadFileToString(argv[1]);
    if (configContent.empty()) {
        return 1; // Exit if file is empty or unreadable
    }
    std::vector<std::string> tokens = Lexer::Tokenize(configContent);

    ConfigParser parser(tokens);
    ConfigTree config;
    try 
    {
        config = parser.Parse();
        // PrintParsedConfig(config);
        
        engine.Init(config);
        engine.Run();
        engine.Shutdown();
    }   
    catch (const std::exception& e)
     {
        std::cerr << "\n\033[1;31m[!] CONFIGURATION ERROR [!]\033[0m\n";
        std::cerr << e.what() << "\n\n";
        return 1;
    }

	

	return 0;
}