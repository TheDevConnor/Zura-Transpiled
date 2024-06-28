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

Node::Stmt *Parser::blockStmt(PStruct *psr) {
    psr->expect(psr, TokenKind::LEFT_BRACE);
    std::vector<Node::Stmt *> stmts;

    while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
        stmts.push_back(parseStmt(psr));
    }

    psr->expect(psr, TokenKind::RIGHT_BRACE);
    return new BlockStmt(stmts);
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

Node::Stmt *Parser::funStmt(PStruct *psr) {
    psr->expect(psr, TokenKind::FUN);
    auto fnName = psr->expect(psr, TokenKind::IDENTIFIER).value;
    psr->expect(psr, TokenKind::LEFT_PAREN);

    std::vector<std::pair<std::string, Node::Type *>> params;
    while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
        auto paramName = psr->expect(psr, TokenKind::IDENTIFIER).value;
        psr->expect(psr, TokenKind::COLON);
        auto paramType = parseType(psr, BindingPower::defaultValue);
        params.push_back({paramName, paramType});

        if (psr->current(psr).kind == TokenKind::COMMA) {
            psr->advance(psr);
        }
    }

    psr->expect(psr, TokenKind::RIGHT_PAREN);
    auto returnType = parseType(psr, BindingPower::defaultValue);
    auto body = blockStmt(psr);

    return new fnStmt(fnName, params, returnType, body);
}

Node::Stmt *Parser::returnStmt(PStruct *psr) {
    psr->expect(psr, TokenKind::RETURN);
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON);
    return new ReturnStmt(expr);
}