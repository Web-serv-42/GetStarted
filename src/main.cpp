/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abnsila <abnsila@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:59:03 by abnsila           #+#    #+#             */
/*   Updated: 2026/05/02 22:36:31 by abnsila          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Webserv.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>


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
	// (void)argc;
	// (void)argv;

	if (argc != 2) {
        std::cout << "Usage: ./webserve <path_to_config_file>" << std::endl;
        return 1;
    }

	std::string configContent = ReadFileToString(argv[1]);
    if (configContent.empty()) {
        return 1; // Exit if file is empty or unreadable
    }

	std::vector<std::string> tokens = Lexer::Tokenize(configContent);

	std::cout << "\n\033[1;36m" << std::string(50, '=') << "\033[0m\n";
    std::cout << "\033[1;32m[+] LEXER OUTPUT (" << tokens.size() << " tokens found) [+]\033[0m\n";
    std::cout << "\033[1;36m" << std::string(50, '-') << "\033[0m\n\n";

    for (size_t i = 0; i < tokens.size(); ++i) 
    {
        // Print each token wrapped in brackets to see exact boundaries
        std::cout << "[" << tokens[i] << "] ";
        
        // Add a line break after semicolons and braces to make it readable in terminal
        if (tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}") {
            std::cout << "\n";
        }
    }

    std::cout << "\n\033[1;36m" << std::string(50, '=') << "\033[0m\n\n";



	// Webserv	engine;

	// engine.Init();
	// engine.Run();
	// engine.Shutdown();

	return 0;
}
