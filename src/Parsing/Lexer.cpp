#include "Parsing/Lexer.hpp"
#include <vector>

Lexer::Lexer() {};
Lexer::~Lexer() {}; 


std::vector<std::string> Lexer::Tokenize(const std::string& configText) 
{
    std::vector<std::string> tokens;
    size_t i = 0;
    size_t length = configText.length();

    while (i < length) 
    {
        char c = configText[i];

        // 1. Skip all whitespaces (spaces, tabs, newlines)
        if (std::isspace(c)) 
        {
            i++;
            continue;
        }

        // 2. Skip comments (ignore everything until a newline)
        if (c == '#') 
        {
            while (i < length && configText[i] != '\n') {
                i++;
            }
            continue;
        }

        // 3. Handle structural characters as independent single-character tokens
        if (c == '{' || c == '}' || c == ';') 
        {
            tokens.push_back(std::string(1, c));
            i++;
            continue;
        }

        // 4. Handle "Words" (Directives, paths, values)
        std::string word = "";
        
        // Keep reading until we hit a space, a structural char, or a comment
        while (i < length && 
               !std::isspace(configText[i]) && 
               configText[i] != '{' && 
               configText[i] != '}' && 
               configText[i] != ';' && 
               configText[i] != '#') 
        {
            word += configText[i];
            i++;
        }
        
        if (!word.empty()) {
            tokens.push_back(word);
        }
    }

    return tokens;
}