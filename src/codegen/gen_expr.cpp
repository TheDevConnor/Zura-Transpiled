#include "../helper/math/math.hpp"
#include "gen.hpp"
#include "optimize.hpp"

#include <optional>

void codegen::visitExpr(Node::Expr *expr) {
  auto handler = lookup(exprHandlers, expr->kind);
  if (handler) {
    handler(expr);
  }
}

// A basic "fallback" type of expression that covers all 1-D, basic expr's
void codegen::primary(Node::Expr *expr) {
  // TODO: Implement the primary expression
  switch (expr->kind) {
  case NodeKind::ND_INT: {
    auto e = static_cast<IntExpr *>(expr);
    push(Instr{.var = PushInstr{.what = "$" + std::to_string(e->value)},
               .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  }
  case NodeKind::ND_IDENT: {
    auto e = static_cast<IdentExpr *>(expr);
    int offset = (stackSize - stackTable.at(e->name)) * 8;

    auto res = (offset == 0) ? "(%rsp)" : std::to_string(offset) + "(%rsp)";

    push(Instr{.var = Comment{.comment = "Clone variable '" + e->name +
                                         "' for use as expr"},
               .type = InstrType::Comment},
         Section::Main);
    push(Instr{.var = PushInstr{.what = res}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  }
  default: {
    std::cerr
        << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
        << std::endl;
    std::cerr << "Primary expression type not implemented! (Type: NodeKind["
              << (int)expr->kind << "])" << std::endl;
    exit(-1);
  }
  }
}

void codegen::binary(Node::Expr *expr) {
  auto e = static_cast<BinaryExpr *>(expr);
  // Visit the left and right expressions

  // NOTE: This will have a setup like "push push, pop pop"
  // which could be optimized into "mov, mov", but that could possibly mess up
  // nested binexpr's and other operations
  visitExpr(e->rhs);
  visitExpr(e->lhs);

  // Pop the right expression
  push(Instr{.var = PopInstr{.where = "%rbx"}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  // Pop the left expression
  push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  // Perform the binary operation
  std::string op = lookup(opMap, e->op);
  if (op == "imul" || op == "idiv") {
    auto instr = (op == "imul") ? InstrType::Mul : InstrType::Div;
    push(Instr{.var = BinaryInstr{.op = op, .src = "%rbx", .dst = "%rax"},
               .type = InstrType::Binary},
         Section::Main);
  } else if (op == "mod") {
    push(Instr{.var = XorInstr{.lhs = "%rdx", .rhs = "%rdx"},
               .type = InstrType::Xor},
         Section::Main);
    push(Instr{.var = DivInstr{.from = "%rbx"}, .type = InstrType::Div},
         Section::Main);
    push(Instr{.var = BinaryInstr{.op = "mov", .src = "%rdx", .dst = "%rax"},
               .type = InstrType::Binary},
         Section::Main);
  } else if (op == "power") {
    // TODO: call native pow func
  } else {
    push(Instr{.var = BinaryInstr{.op = op, .src = "%rax", .dst = "%rbx"},
               .type = InstrType::Binary},
         Section::Main);
  }

  // Push the result of the binary operation
//   push(Instr{.var = PushInstr{.what = "%rax"}, .type = InstrType::Push},
//        Section::Main);
//   stackSize++;
}

void codegen::unary(Node::Expr *expr) {
  auto e = static_cast<UnaryExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Unary expression not implemented! (Op: " << e->op
            << ", Expr: " << e->expr->kind << ")" << std::endl;
  exit(-1);
}

void codegen::grouping(Node::Expr *expr) {
  auto e = static_cast<GroupExpr *>(expr);
  // Visit the expression inside the grouping
  visitExpr(e->expr);

  // Pop the expression inside the grouping
  push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  // Push the expression inside the grouping
  push(Instr{.var = PushInstr{.what = "%rax"}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

void codegen::call(Node::Expr *expr) {
  auto e = static_cast<CallExpr *>(expr);
  auto n = static_cast<IdentExpr *>(e->callee);
  // Push each argument one by one.
  for (auto p : e->args) {
    // evaluate them
    codegen::visitExpr(p);
  }
  // Call the function
  push(Instr{.var = CallInstr{.name = "usr_" + n->name},
             .type = InstrType::Call,
             .optimize = false},
       Section::Main);
  push(Instr{.var = PushInstr{.what = "%rax", .whatSize = DataSize::Qword},
             .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

void codegen::ternary(Node::Expr *expr) {
  auto e = static_cast<TernaryExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Ternary expression not implemented!" << std::endl;
  exit(-1);
}

void codegen::assign(Node::Expr *expr) {
  auto e = static_cast<AssignmentExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Assign expression not implemented! (New value: NodeKind["
            << (int)e->rhs->kind << "], Assignee: NodeKind["
            << (int)e->assignee->kind << "])" << std::endl;
  exit(-1);
}

void codegen::_arrayExpr(Node::Expr *expr) {
  auto e = static_cast<ArrayExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Array expression not implemented! (Type: NodeKind["
            << (int)e->type->kind << "])" << std::endl;
  exit(-1);
}

void codegen::arrayElem(Node::Expr *expr) {
  auto e = static_cast<IndexExpr *>(expr);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Array element not implemented! (LHS: NodeKind["
            << (int)e->lhs->kind << "], RHS: NodeKind[" << (int)e->rhs->kind
            << "])" << std::endl;
  exit(-1);
}