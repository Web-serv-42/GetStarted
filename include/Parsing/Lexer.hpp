#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

class Lexer {
    public:
        static std::vector<std::string> Tokenize(const std::string& configFile);

    private:
        Lexer();
        ~Lexer();
};

#endif