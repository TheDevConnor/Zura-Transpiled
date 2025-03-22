// library for log2 function (you will see where this is used)
#include <cmath>
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
    // NOTE: PLEASE COME BACK HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
      IntExpr *temp = new IntExpr(expr->line, expr->pos, -value, expr->file_id); // (Somehow, -10 in plain source code is read as Unary - of 10 rather than an int of -10)
      temp->isUnsigned = false; // We ARE signed now!
      return temp;
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
    }
  }
  if (lhs->kind == ND_INT && op == "/") {
    IntExpr *rhsExpr = static_cast<IntExpr *>(rhs);
    int rhsVal = rhsExpr->value;
    double shiftAmount = log2(rhsVal); // If this number is a whole number, it is a true power of 2
    
    // Right shifting still works even with a negative number; so I removed that check!
    if (shiftAmount == floor(shiftAmount)) {
      BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, lhs, new IntExpr(rhsExpr->line, rhsExpr->pos, (long long)shiftAmount, rhsExpr->file_id), ">>", expr->file_id);
      temp->asmType = lhs->asmType;
      return temp;
    }
    // check the value of the lhs
    // if it was 0, the answer is also 0
    // 0 / 2 = 0
    // 0 / 17 = 0
    if (static_cast<IntExpr *>(lhs)->value == 0) {
      return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
    }
  }
  if (op == "*") {
    // check if either side was a useless calculation (0 * x, 1 * x)
    if (lhs->kind == ND_INT) {
      IntExpr *lhsExpr = static_cast<IntExpr *>(lhs);
      int lhsVal = lhsExpr->value;
      if (lhsVal == 0) {
        return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
      }
      if (lhsVal == 1) {
        return rhs;
      }
      double shiftAmount = log2(lhsVal); // If this number is a whole number, it is a true power of 2
      if (shiftAmount == floor(shiftAmount)) {
        BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, rhs, new IntExpr(lhsExpr->line, lhsExpr->pos, (long long)shiftAmount, lhsExpr->file_id), "<<>>>>", expr->file_id);
        temp->asmType = rhs->asmType;
        return temp;
      }
    }
    if (rhs->kind == ND_INT) {
      IntExpr *rhsExpr = static_cast<IntExpr *>(rhs);
      int rhsVal = rhsExpr->value;
      if (rhsVal == 0) {
        return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
      }
      if (rhsVal == 1) {
        return lhs;
      }
      double shiftAmount = log2(rhsVal); // If this number is a whole number, it is a true power of 2
      if (shiftAmount == floor(shiftAmount)) {
        BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, rhs, new IntExpr(rhsExpr->line, rhsExpr->pos, (long long)shiftAmount, rhsExpr->file_id), "<<>>>>", expr->file_id);
        temp->asmType = rhs->asmType;
        return temp;
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
        return expr;
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
    // check if it is the same operation on both sides
    // x + x = 2x
    // x - x = 0
    if (lhs->kind == ND_IDENT && rhs->kind == ND_IDENT) {
      IdentExpr *lhsIdent = static_cast<IdentExpr *>(lhs);
      IdentExpr *rhsIdent = static_cast<IdentExpr *>(rhs);
      if (lhsIdent->name == rhsIdent->name) {
        if (op == "-") return new IntExpr(expr->line, expr->pos, 0, expr->file_id);
        if (op == "+") {
          // Check if the type is unsigned (this could be FURTHER optimized into a << )
          if (static_cast<SymbolType *>(lhs->asmType)->signedness != SymbolType::Signedness::SIGNED) {
            BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, lhsIdent, new IntExpr(rhsIdent->line, rhsIdent->pos, 1, rhsIdent->file_id), "<<", expr->file_id);
            temp->asmType = lhs->asmType;
            return temp;
          }
          // Just do a *2
          return new BinaryExpr(expr->line, expr->pos, lhsIdent, new IntExpr(rhsIdent->line, rhsIdent->pos, 2, rhsIdent->file_id), "*", expr->file_id);
        }
      }
    }
  }


  // Check for MORE int literal math
  // Ex: x + 4 + 4 + 4 -> (((x + 4) + 4) + 4) which cant be analyzed
  if (lhs->kind == ND_INT && rhs->kind == ND_BINARY) {
    BinaryExpr *rhsBin = static_cast<BinaryExpr *>(rhs);
    if (rhsBin->lhs->kind == ND_INT) {
      long long lhsLhsVal = static_cast<IntExpr *>(lhs)->value;
      long long rhsLhsVal = static_cast<IntExpr *>(rhsBin->lhs)->value;
      long long result = 0;
      // ex: 4 + x + 4 -> (4 + x) + 4 -> 8 + x
      // ex: 4 - x + 4 -> (4 - x) + 4 -> x
      if (op == "+") result = lhsLhsVal + rhsLhsVal;
      if (op == "*") result = lhsLhsVal * rhsLhsVal;
      if (op == "-") result = lhsLhsVal - rhsLhsVal;
      BinaryExpr *temp =  new BinaryExpr(expr->line, expr->pos, new IntExpr(expr->line, expr->pos, result, expr->file_id), rhsBin->rhs, rhsBin->op, expr->file_id);
      temp->asmType = rhs->asmType;
      return temp;
    }
    // same thing, other side
    if (rhsBin->rhs->kind == ND_INT) {
      long long lhsRhsVal = static_cast<IntExpr *>(lhs)->value;
      long long rhsRhsVal = static_cast<IntExpr *>(rhsBin->rhs)->value;
      long long result = 0;
      // ex: 4 + 4 + x -> (4 + 4) + x -> 8 + x
      // ex: 4 + 4 - x -> (4 + 4) - x -> 8 - x
      if (op == "+") result = lhsRhsVal + rhsRhsVal;
      if (op == "*") result = lhsRhsVal * rhsRhsVal;
      if (op == "-") result = lhsRhsVal - rhsRhsVal;
      BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, rhsBin->lhs, new IntExpr(expr->line, expr->pos, result, expr->file_id), rhsBin->op, expr->file_id);
      temp->asmType = rhs->asmType;
      return temp;
    }
  }
  if (lhs->kind == ND_BINARY && rhs->kind == ND_INT) {
    BinaryExpr *lhsBin = static_cast<BinaryExpr *>(lhs);
    if (lhsBin->lhs->kind == ND_INT) {
      long long lhsLhsVal = static_cast<IntExpr *>(lhsBin->lhs)->value;
      long long rhsVal = static_cast<IntExpr *>(rhs)->value;
      long long result = 0;
      // ex: x + 4 + 4 -> (x + 4) + 4 -> x + 8
      // ex: x - 4 + 4 -> (x - 4) + 4 -> x
      if (op == "+") result = lhsLhsVal + rhsVal;
      if (op == "*") result = lhsLhsVal * rhsVal;
      if (op == "-") result = lhsLhsVal - rhsVal;
      BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, new IntExpr(expr->line, expr->pos, result, expr->file_id), lhsBin->rhs, lhsBin->op, expr->file_id);
      temp->asmType = lhs->asmType;
      return temp;
    }
    // same thing, other side
    if (lhsBin->rhs->kind == ND_INT) {
      long long lhsRhsVal = static_cast<IntExpr *>(lhsBin->rhs)->value;
      long long rhsVal = static_cast<IntExpr *>(rhs)->value;
      long long result = 0;
      // ex: 4 + x + 4 -> (4 + x) + 4 -> 8 + x
      // ex: 4 - x + 4 -> (4 - x) + 4 -> 4 + 4
      if (op == "+") result = lhsRhsVal + rhsVal;
      if (op == "*") result = lhsRhsVal * rhsVal;
      if (op == "-") result = lhsRhsVal - rhsVal;
      BinaryExpr *temp = new BinaryExpr(expr->line, expr->pos, lhsBin->lhs, new IntExpr(expr->line, expr->pos, result, expr->file_id), lhsBin->op, expr->file_id);
      temp->asmType = lhs->asmType;
      return temp;
    } 
  }
  // We've made all the optimizations we can (for now)

  return expr;
};