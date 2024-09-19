#include "gen.hpp"
#include "optimize.hpp"
// #include <cstddef>

void codegen::visitExpr(Node::Expr *expr) {
  auto handler = lookup(exprHandlers, expr->kind);
  if (handler) {
    handler(expr);
  }
}

void codegen::binary(Node::Expr *expr) {
     auto e = static_cast<BinaryExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Binary expression not implemented! (Op: " << e->op << "LHS: NodeKind[" << (int)e->lhs->kind << "], RHS: NodeKind[" << (int)e->rhs->kind << "])" << std::endl;
     exit(-1);
}

void codegen::grouping(Node::Expr *expr) {
     auto e = static_cast<GroupExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Group expression not implemented! (Inner: NodeKind[" << (int)e->expr->kind << "])" << std::endl;
     exit(-1);
}

void codegen::unary(Node::Expr *expr) {
     auto e = static_cast<UnaryExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Unary expression not implemented! (Op: " << e->op << ", Expr: " << e->expr->kind << ")" << std::endl;
     exit(-1);
}

void codegen::call(Node::Expr *expr) {
     auto e = static_cast<CallExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Call expression not implemented! (Name: " << e->callee << ")" << std::endl;
     exit(-1);
}

void codegen::ternary(Node::Expr *expr) {
     auto e = static_cast<TernaryExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Ternary expression not implemented!" << std::endl;
     exit(-1);
}

void codegen::assign(Node::Expr *expr) {
     auto e = static_cast<AssignmentExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Assign expression not implemented! (New value: NodeKind[" << (int)e->rhs->kind << "], Assignee: NodeKind[" << (int)e->assignee->kind << "])" << std::endl;
     exit(-1);
}

void codegen::primary(Node::Expr *expr) {
     // No need to cast for this, this is basically an umbrella for all of the lowest-level expressions
     // String
     // Integer
     // Float
     // Boolean
     switch (expr->kind) {
          case NodeKind::ND_INT: {
               auto e = static_cast<IntExpr *>(expr);
               push(Instr {.var=PushInstr{.what='$' + std::to_string(e->value)},.type=InstrType::Push},Section::Main);
               stackSize++;
               break;
          }
          case NodeKind::ND_FLOAT: {
               auto e = static_cast<FloatExpr *>(expr);
               std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
               std::cerr << "Floats not implemented! (Value: " << e->value << ")" << std::endl;
               exit(-1);
          }
          default: {
               std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
               std::cerr << "Primary expression not implemented! (Type: NodeKind[" << (int)expr->kind << "])" << std::endl;
               exit(-1);
          }
     }
}

void codegen::_arrayExpr(Node::Expr *expr) {
     auto e = static_cast<ArrayExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Array expression not implemented! (Type: NodeKind[" << (int)e->type->kind << "])" << std::endl;
     exit(-1);
}

void codegen::arrayElem(Node::Expr *expr) {
     auto e = static_cast<IndexExpr *>(expr);
     std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
     std::cerr << "Array element not implemented! (LHS: NodeKind[" << (int)e->lhs->kind << "], RHS: NodeKind[" << (int)e->rhs->kind << "])" << std::endl;
     exit(-1);
}