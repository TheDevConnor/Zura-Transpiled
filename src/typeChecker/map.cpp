#include "../ast/ast.hpp"
#include "type.hpp"

#include <algorithm>
#include <functional>
#include <vector>

void TypeChecker::lookup(callables_table &ctable, symbol_table &table, Node::Stmt *stmt) {
    auto it = std::find_if(stmts.begin(), stmts.end(), [stmt](auto &s) {
        return s.first == stmt->kind;
    });

    if (it != stmts.end()) {
        it->second(ctable, table, stmt); 
    }
}

void TypeChecker::lookup(callables_table &ctable, symbol_table &table, Node::Expr *expr) {
    auto it = std::find_if(exprs.begin(), exprs.end(), [expr](auto &e) {
        return e.first == expr->kind;
    });

    if (it != exprs.end()) {
        it->second(ctable, table, expr); 
    }
}