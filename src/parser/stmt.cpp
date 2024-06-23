#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

Node::Stmt *Parser::parseStmt(PStruct *psr) {
    // TODO: Implement a loop to parse all statements and handle the lookup properly
    // auto stmt_it = stmt(psr);

    return exprStmt(psr);
}

Node::Stmt *Parser::exprStmt(PStruct *psr) {
    auto *expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON);
    return new ExprStmt(expr);
}

/// Parses a variable statement
/// This is an example of a var in zura:
/// have x : i8 = 10;
Node::Stmt *Parser::varStmt(PStruct *psr) {
    psr->expect(psr, TokenKind::VAR);
    auto name = (psr->expect(psr, TokenKind::IDENTIFIER)) 
                ? psr->peek(psr, -1).value
                : "";

    auto type = (psr->expect(psr, TokenKind::COLON)) 
                ? psr->peek(psr, 0).value
                : "";

    psr->advance(psr);

    psr->expect(psr, TokenKind::EQUAL);
    auto *expr = parseExpr(psr, BindingPower::defaultValue);
    psr->advance(psr);
    psr->expect(psr, TokenKind::SEMICOLON);

    return new VarStmt(name, type, new ExprStmt(expr));
}
