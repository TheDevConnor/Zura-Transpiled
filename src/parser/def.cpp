#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "parser.hpp"

bool Parser::match(TokenKind kinds) {
    if (currentToken.kind != kinds) return false;

    advance();
    return true;
}

void Parser::advance() {
    previousToken = currentToken;

    while(true) {
        currentToken = lexer.scanToken();
        if (currentToken.kind != TokenKind::ERROR_) break;

        hadError = true;
        ParserError::error(currentToken, "Unexpected character", lexer);
    }
}

void Parser::consume(TokenKind kind, std::string message) {
    if (currentToken.kind == kind) {
        advance();
        return;
    }

    hadError = true;
    ParserError::error(currentToken, message, lexer);
}

void Parser::synchronize() {
    advance();

    while (currentToken.kind != TokenKind::END_OF_FILE) {
        if (previousToken.kind == TokenKind::SEMICOLON) return;

        switch (currentToken.kind) {
            case TokenKind::CLASS:
            case TokenKind::FUN:
            case TokenKind::VAR:
            case TokenKind::IF:
            case TokenKind::LOOP:
            case TokenKind::PRINT:
            case TokenKind::RETURN:
                return;
            default:
                break;
        }

        advance();
    }
}

int Parser::getPrecedence() {
    switch (currentToken.kind) {
        case TokenKind::PLUS:
        case TokenKind::MINUS:
            return 1;
        case TokenKind::STAR:
        case TokenKind::SLASH:
        case TokenKind::MODULO:
            return 2;
        default:
            return -1;
    }
}