#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

Node::Stmt *Parser::parseStmt(PStruct *psr) {
    auto stmt_it = stmt(psr);

    if (stmt_it != nullptr)
        return stmt_it;

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
    auto varName = psr->expect(psr, TokenKind::IDENTIFIER).value;

    psr->expect(psr, TokenKind::COLON);
    auto varType = parseType(psr, BindingPower::defaultValue); 

    psr->expect(psr, TokenKind::EQUAL);
    auto assignedValue = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON);

    return new VarStmt(varName, varType, new ExprStmt(assignedValue));
}
