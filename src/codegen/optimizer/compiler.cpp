#include "compiler.hpp"
#include "../gen.hpp"

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
    case NodeKind::ND_UNARY:  return optimizeUnary(static_cast<UnaryExpr *>(expr));
    default: return expr;
  }
}

Node::Expr *CompileOptimizer::optimizeUnary(UnaryExpr *expr) {
  Node::Expr *operand = CompileOptimizer::optimizeExpr(expr->expr);
  if (operand->kind == ND_INT) {
    int value = static_cast<IntExpr *>(operand)->value;
    if (expr->op == "~") { // binary not each bit
      return new IntExpr(expr->line, expr->pos, ~value, expr->file_id);
    }
    if (expr->op == "-") { // negation
      return new IntExpr(expr->line, expr->pos, -value, expr->file_id); // (Somehow, -10 in plain source code is read as Unary - of 10 rather than an int of -10)
    }
  }
  if (operand->kind == ND_BOOL) {
    bool value = static_cast<BoolExpr *>(operand)->value;
    if (expr->op == "!") { // logical not
      return new BoolExpr(expr->line, expr->pos, !value, expr->file_id);
    }
    // ... other unary bools (if they exist anyway)
  }
  if (operand->kind == ND_BINARY) {
    BinaryExpr *binExpr = static_cast<BinaryExpr *>(operand);
    // check if the binary expression is a comparison
    if (boolOperations.contains(binExpr->op)) {
      // if it is, we can optimize the unary expression
      if (expr->op == "!") {
        // we can optimize the comparison's operation rather than get the opposite afterwards
        std::string newOp = "";
        if (binExpr->op == ">") newOp = "<=";
        if (binExpr->op == "<") newOp = ">=";
        if (binExpr->op == ">=") newOp = "<";
        if (binExpr->op == "<=") newOp = ">";
        if (binExpr->op == "==") newOp = "!=";
        if (binExpr->op == "!=") newOp = "==";
        // || and && are not really comparisons (they dont have opposites that are readily available- there is no 'nor' or 'nand')
        if (newOp != "")
          return new BinaryExpr(expr->line, expr->pos, binExpr->lhs, binExpr->rhs, newOp, expr->file_id);
        // If it was not changed, fall through and return the intended expression
      }
    }
  }
  return expr;
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
    // check the value of the lhs
    // if it was 0, the answer is also 0
    if (static_cast<IntExpr *>(lhs)->value == 0) {
      return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
    }
    return expr;
  }
  if (op == "*") {
    // check if either side was a useless calculation (0 * x, 1 * x)
    if (lhs->kind == ND_INT) {
      int lhsVal = static_cast<IntExpr *>(lhs)->value;
      if (lhsVal == 0) {
        return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
      }
      if (lhsVal == 1) {
        return rhs;
      }
    }
    if (rhs->kind == ND_INT) {
      int rhsVal = static_cast<IntExpr *>(rhs)->value;
      if (rhsVal == 0) {
        return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
      }
      if (rhsVal == 1) {
        return lhs;
      }
    }
  }
  if (op == "/") {
    // check if the operation was useless (x / 1 or error x / 0)
    if (rhs->kind == ND_INT) {
      int rhsVal = static_cast<IntExpr *>(rhs)->value;
      if (rhsVal == 1) {
        return lhs;
      }
      if (rhsVal == 0) {
        std::string msg = "Dividing by zero is not allowed!";
        codegen::handleError(expr->line, expr->pos, msg, "Compile Error");
      }
    }
    // TODO: Check if dividing by an even 1/x float to try optimizing into a Multiply
  }
  if (op == "+" || op == "-") {
    // check if useless (one of the sides is 0)
    if (lhs->kind == ND_INT) {
      int lhsVal = static_cast<IntExpr *>(lhs)->value;
      if (lhsVal == 0) return rhs;
    }
  }

  // We've made all the optimizations we can (for now)

  return expr;
};