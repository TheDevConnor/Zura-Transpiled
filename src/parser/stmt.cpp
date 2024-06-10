#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

Node::Stmt *ParserNamespace::exprStmt(Parser *psr) {
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    return new ExprStmt(expr);
}

Node::Stmt *ParserNamespace::varStmt(Parser *psr) {
    // TODO: Implement varStmt
    return nullptr;
}
