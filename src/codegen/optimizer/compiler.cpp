#include "compiler.hpp"

Node::Stmt *CompileOptimizer::optimizeStmt(Node::Stmt *stmt) {
  // May be one day used for optimizations such as...
  // If jumping (automatic switch statement maker)
  // Remove unused functions and variables
  // Remove dead or unreachable code (if statements that are always false, code after returns)
  // Remove redundant code (such as a = a)
  // What one day might be...
  switch (stmt->kind) {
    case ND_IF_STMT: return optimizeIfStmt(static_cast<IfStmt *>(stmt));
    default:         return stmt;
  }
}

Node::Stmt *CompileOptimizer::optimizeIfStmt(IfStmt *stmt) {
  Node::Expr *realConditionExpr = optimizeExpr(stmt->condition);
  // If the condition is a bool literal..
  if (realConditionExpr->kind == ND_BOOL) {
    // Bool literals? why would the dev ever wanna do this anyway?
    // Who cares, man!!
    bool value = static_cast<BoolExpr *>(realConditionExpr)->value;
    if (value) {
      return optimizeStmt(stmt->thenStmt);
    }
    if (stmt->elseStmt != nullptr) {
      return optimizeStmt(stmt->elseStmt);
    } else {
      return nullptr;
    }
  }
  
  // That's realistically the only optimization possible.
  // That and jump tables. I'm scared of those!
  return stmt;
};

Node::Expr *CompileOptimizer::optimizeExpr(Node::Expr *expr) {
  switch (expr->kind) {
    case NodeKind::ND_BINARY: return optimizeBinary(static_cast<BinaryExpr *>(expr));
    case NodeKind::ND_GROUP:  return optimizeExpr(static_cast<GroupExpr *>(expr)->expr);
    default: return expr;
  }
}

Node::Expr *CompileOptimizer::optimizeBinary(BinaryExpr *expr) {
  // Check if int
  Node::Expr *lhs = CompileOptimizer::optimizeExpr(expr->lhs); // If the lhs is a binExpr, that will be optimized too!!
  Node::Expr *rhs = CompileOptimizer::optimizeExpr(expr->rhs);
  std::string op = expr->op;
  if (lhs->kind == ND_INT && rhs->kind == ND_INT) {
    int lhsVal = static_cast<IntExpr *>(lhs)->value;
    int rhsVal = static_cast<IntExpr *>(rhs)->value;
    // Check if the operation is a comparison (||, &&, ==, !=, <, >, <=, >=)
    if (boolOperations.contains(op)) {
      // Evaluate the expression
      bool result = false;
      if (op == "||") result = lhsVal || rhsVal;
      if (op == "&&") result = lhsVal && rhsVal;
      if (op == "==") result = lhsVal == rhsVal;
      if (op == "!=") result = lhsVal != rhsVal;
      if (op == "<") result = lhsVal < rhsVal;
      if (op == ">") result = lhsVal > rhsVal;
      if (op == "<=") result = lhsVal <= rhsVal;
      if (op == ">=") result = lhsVal >= rhsVal;
      return new BoolExpr(expr->line, expr->pos, result, expr->file_id);
    } else if (intOperations.contains(op)) {
      long long result = 0;
      if (op == "+") result = lhsVal + rhsVal;
      if (op == "-") result = lhsVal - rhsVal;
      if (op == "*") result = lhsVal * rhsVal;
      if (op == "/") result = lhsVal / rhsVal;
      if (op == "%") result = lhsVal % rhsVal;
      if (op == "&") result = lhsVal & rhsVal;
      if (op == "|") result = lhsVal | rhsVal;
      if (op == "^") result = lhsVal ^ rhsVal;
      if (op == "<<") result = lhsVal << rhsVal;
      if (op == ">>") result = lhsVal >> rhsVal;
      return new IntExpr(expr->line, expr->pos, result, expr->file_id);
    } else {
      return expr;
    }
  }
  if (lhs->kind == ND_INT && op == "/") {
    int rhsVal = static_cast<IntExpr *>(rhs)->value;
    if (rhsVal == 2) {
      return new BinaryExpr(expr->line, expr->pos, lhs, rhs, ">>", expr->file_id);
    }
    return expr;
  }

  if (lhs->kind == ND_FLOAT && rhs->kind == ND_FLOAT) {
    float lhsVal = static_cast<FloatExpr *>(lhs)->value;
    float rhsVal = static_cast<FloatExpr *>(rhs)->value;
    float result = 0;
    if (op == "+") result = lhsVal + rhsVal;
    if (op == "-") result = lhsVal - rhsVal;
    if (op == "*") result = lhsVal * rhsVal;
    if (op == "/") result = lhsVal / rhsVal;
    return new FloatExpr(expr->line, expr->pos, result, expr->file_id);
  };
  return expr;
};