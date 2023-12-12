#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

AstNode* Parser::declaration() {
    if (match(TokenKind::VAR)) return varDeclaration();
    return statement();
}

AstNode* Parser::statement() {
    if (match(TokenKind::PRINT)) return printStatement();
    return expressionStatement();
}

AstNode* Parser::expressionStatement() {
    AstNode* expr = expression();
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return new AstNode(AstNodeType::EXPRESSION, new AstNode::Stmt{ AstNode::Expression(expr) });
}

AstNode* Parser::printStatement() {
    AstNode* expr = expression();
    consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    return new AstNode(AstNodeType::PRINT, new AstNode::Print(expr));
}

AstNode* Parser::varDeclaration() {
    consume(TokenKind::IDENTIFIER, "Expected variable name");

    Lexer::Token name = previousToken;

    AstNode* type = nullptr;
    if (match(TokenKind::LESS)) {
        switch (currentToken.kind) {
            case I8:
            case I16:
            case I32:
            case I64: 
                type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken)); break;
            case U8:
            case U16:
            case U32:
            case U64: 
                type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken)); break;
            case F32:
            case F64: 
                type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken)); break;
            case Bool: 
                type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken)); break;
            case STRING: 
                type = new AstNode(AstNodeType::TYPE, new AstNode::Type(currentToken)); break;
        default:
            consume(currentToken.kind, "Expected type annotation of either i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool or string");
        }
        advance();
        consume(TokenKind::GREATER, "Expected '>' after type annotation");
    }

    consume(TokenKind::EQUAL, "Expected '=' after variable name and type annotation");

    AstNode* initializer = expression();

    consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
    return new AstNode(AstNodeType::VAR_DECLARATION, new AstNode::VarDeclaration(name, type, initializer));
}

