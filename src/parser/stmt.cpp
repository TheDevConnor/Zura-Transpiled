#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

using namespace ParserClass;

Node::Stmt *ParserClass::exprStmt(Parser *psr) {
    auto expr = parseExpr(psr, BindingPower::defaultValue);

    expr->debug();
    std::cout << std::endl;

    // TODO: Add semicolon check after expression is working properly
    return new ExprStmt(expr);
}
