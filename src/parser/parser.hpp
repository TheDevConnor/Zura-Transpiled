#pragma once

#include <vector>
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

class Parser {
public:
    Parser(Lexer& lexer);

    AstNode* parse();

private:
    Lexer& lexer;
    Lexer::Token currentToken;

    AstNode* expression(int precedence = 0);
    AstNode* unary();
    // AstNode* binary(AstNode* left, int precedence);
    AstNode* grouping();
    AstNode* literal();

    bool match(std::vector<TokenKind> kinds);
    bool check(std::vector<TokenKind> kinds);
    Lexer::Token consume(TokenKind kind, std::string message);
    void synchronize();

    int getPrecedence();
};