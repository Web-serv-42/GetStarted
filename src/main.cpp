/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/07/26 10:42:32 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Server/Webserv.hpp"
#include "Parsing/Lexer.hpp"
#include "HTTP/Request/Request.hpp"
#include "Parsing/ConfigParser.hpp"
#include "Parsing/RequestParser.hpp"

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

int main(int argc, char const *argv[])
{
    if (argc > 2) {
        std::cerr << "Usage: ./webserve <path_to_config_file>" << std::endl;
        return 1;
    }

    std::string configContent;
	Webserv     engine;

    if (argc == 1)
        configContent = ReadFileToString("./config/mandatory.conf");
    else
        configContent = ReadFileToString(argv[1]);
    if (configContent.empty()) {
        return 1;
    }
    std::vector<std::string> tokens = Lexer::Tokenize(configContent);

    ConfigParser parser(tokens);
    ConfigTree config;
    try 
    {
        config = parser.Parse();
        // PrintParsedConfig(config);
        
        bool    success = engine.Init(config);
        if (success)
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