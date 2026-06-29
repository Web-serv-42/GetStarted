/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablabib <ablabib@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/06/29 20:12:20 by ablabib          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../include/Parsing/Lexer.hpp"
#include "../include/Parsing/ConfigParser.hpp"

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
    std::cout << "\n\033[1;35m" << std::string(60, '=') << "\033[0m\n";
    std::cout << "\033[1;33m[+] PARSER OUTPUT (" << tree.servers.size() << " Servers Found) [+]\033[0m\n";
    std::cout << "\033[1;35m" << std::string(60, '-') << "\033[0m\n\n";

    for (size_t i = 0; i < tree.servers.size(); ++i) 
    {
        std::cout << "\033[1;32m=== SERVER " << (i + 1) << " ===\033[0m\n";
        
        // 1. Print Server-Level Directives
        std::map<std::string, std::string>::const_iterator sIt;
        for (sIt = tree.servers[i].directives.begin(); sIt != tree.servers[i].directives.end(); ++sIt) 
        {
            std::cout << "  " << sIt->first << " : \033[0;36m" << sIt->second << "\033[0m\n";
        }

        // 2. Print Location Blocks inside this Server
        for (size_t j = 0; j < tree.servers[i].locations.size(); ++j) 
        {
            const LocationConfig& loc = tree.servers[i].locations[j];
            std::cout << "\n  \033[1;34m--- Location: " << loc.path << " ---\033[0m\n";
            
            // Print Location-Level Directives
            std::map<std::string, std::string>::const_iterator lIt;
            for (lIt = loc.directives.begin(); lIt != loc.directives.end(); ++lIt) 
            {
                std::cout << "      " << lIt->first << " : \033[0;36m" << lIt->second << "\033[0m\n";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\033[1;35m" << std::string(60, '=') << "\033[0m\n\n";
}

int main(int argc, char** argv) 
{
    if (argc != 2) {
        std::cerr << "Usage: ./webserve <path_to_config_file>" << std::endl;
        return 1;
    }

    // 1. Read the file
    std::string configContent = ReadFileToString(argv[1]);
    if (configContent.empty()) {
        return 1; // Exit if file is empty or unreadable
    }

    try 
    {
        // 2. Run the Lexer
        std::vector<std::string> tokens = Lexer::Tokenize(configContent);
        
        // Uncomment this block if you still want to see the raw Lexer output
        // std::cout << "\n\033[1;36m" << std::string(50, '=') << "\033[0m\n";
        // std::cout << "\033[1;32m[+] LEXER OUTPUT (" << tokens.size() << " tokens found) [+]\033[0m\n";
        // for (size_t i = 0; i < tokens.size(); ++i) {
        //     std::cout << "[" << tokens[i] << "] ";
        //     if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}") std::cout << "\n";
        // }
        // std::cout << "\033[1;36m" << std::string(50, '=') << "\033[0m\n\n";
        

        // 3. Run the Parser
        ConfigParser parser(tokens);
        ConfigTree config = parser.Parse();

        // 4. Visualize the Result!
        PrintParsedConfig(config);
    } 
    // 5. Catch and print any syntax errors safely
    catch (const std::exception& e) 
    {
        std::cerr << "\n\033[1;31m[!] CONFIGURATION ERROR [!]\033[0m\n";
        std::cerr << e.what() << "\n\n";
        return 1;
    }

    return 0;
}