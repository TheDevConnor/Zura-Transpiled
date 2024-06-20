#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

Node::Stmt *ParserNamespace::parseStmt(Parser *psr) {
    std::cout << "Parsing statement\n";
    auto stmt_it = stmt_lu[psr->current(psr).kind];

    return exprStmt(psr);
}

Node::Stmt *ParserNamespace::exprStmt(Parser *psr) {
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON);
    return new ExprStmt(expr);
}

/// Parses a variable statement
/// This is an example of a var in zura:
/// have x : i8 = 10;
Node::Stmt *ParserNamespace::varStmt(Parser *psr) {
    psr->expect(psr, TokenKind::VAR);
    auto name = (psr->expect(psr, TokenKind::IDENTIFIER)) 
                ? psr->peek(psr, -1).value
                : "";

    auto type = (psr->expect(psr, TokenKind::COLON)) 
                ? psr->peek(psr, 0).value
                : "";

    psr->advance(psr);

    psr->expect(psr, TokenKind::EQUAL);
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    psr->advance(psr);
    psr->expect(psr, TokenKind::SEMICOLON);

    return new VarStmt(name, type, new ExprStmt(expr));
}
