#include "gen.hpp"

void codegen::visitExpr(Node::Expr *expr) {
  auto handler = lookup(exprHandlers, expr->kind);
  if (handler) {
    handler(expr);
  }
}

void codegen::binary(Node::Expr *expr) {
  auto binary = static_cast<BinaryExpr *>(expr);
  visitExpr(binary->lhs);
  push("\tpop rbx", true); // pop lhs
  stackSize++;
  visitExpr(binary->rhs);
  push("\tpop rcx", true); // pop rhs
  stackSize++;

  switch (binary->op[0]) {
  case '+':
    push("\tadd rbx, rcx", true);
    push("\tmov rdi, rbx", true);
    break;
  default:
    break;
  }
}

void codegen::unary(Node::Expr *expr) {}

void codegen::call(Node::Expr *expr) {}

void codegen::ternary(Node::Expr *expr) {}

void codegen::primary(Node::Expr *expr) {
  switch (expr->kind) {
  case ND_NUMBER: {
    auto number = static_cast<NumberExpr *>(expr);
    auto res = (number->value == (int)number->value)
                   ? std::to_string((int)number->value)
                   : std::to_string(number->value);
    push("\tpush " + res, true);
    stackSize++;
    break;
  }
  case ND_IDENT: {
    auto ident = static_cast<IdentExpr *>(expr);

    size_t offset = (stackSize - stackTable.at(ident->name)) * 8;
    if (offset == 0) {
      push("\tpush qword [rsp]", true);
      stackSize++;
    } else {
      push("\tpush qword [rsp + " + std::to_string(offset) + "]", true);
      stackSize++;
    }
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    push("\tpush " + string->value, true);
    stackSize++;
    break;
  }
  case ND_BOOL: {
    auto boolean = static_cast<BoolExpr *>(expr);
    push("\tpush " + std::to_string(boolean->value), true);
    stackSize++;
    break;
  }
  default:
    break;
  }
}
