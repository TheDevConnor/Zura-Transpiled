#include "gen.hpp"

void codegen::visitExpr(Node::Expr *expr) {
  auto handler = lookup(exprHandlers, expr->kind);
  if (handler) {
    handler(expr);
  }
}

void codegen::binary(Node::Expr *expr) {}

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
    push("push " + res, true);
    break;
  }
  case ND_IDENT: {
    auto ident = static_cast<IdentExpr *>(expr);
    push("push " + ident->name, true);
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    push("push " + string->value, true);
    break;
  }
  case ND_BOOL: {
    auto boolean = static_cast<BoolExpr *>(expr);
    push("push " + std::to_string(boolean->value), true);
    break;
  }
  default:
    break;
  }
}