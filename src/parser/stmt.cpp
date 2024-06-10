#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

Node::Stmt *ParserNamespace::exprStmt(Parser *psr) {
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    return new ExprStmt(expr);
}
