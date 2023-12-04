#pragma once

#include "../lexer/lexer.hpp"

class Parser {
public:
    Parser(Lexer::Token token, bool error) : token(token) {}
    ~Parser() {}

    struct ParserStruct {
        Lexer::Token previous;
        Lexer::Token token;

        bool hadError = false;
        bool panicMode = false;
    };

    ParserStruct parser;
private:
    Lexer::Token token;
};