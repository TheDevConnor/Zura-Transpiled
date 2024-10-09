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
                                         "' for use as expr (offset: " +
                                         std::to_string(offset) + ")"},
               .type = InstrType::Comment},
         Section::Main);

    // Push the instruction to use the calculated stack location
    push(Instr{.var = PushInstr{.what = res}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    std::string label = "string" + std::to_string(stringCount);

    // Push the label onto the stack
    push(Instr{.var =
                   PushInstr{.what = '$' + label, .whatSize = DataSize::Qword},
               .type = InstrType::Push},
         Section::Main);
    stackSize++;

    // define the string in the data section
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::Data);

    // Push the string onto the data section
    push(Instr{.var = AscizInstr{.what=string->value},
               .type = InstrType::Asciz},
         Section::Data);
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
  bool isAddition = (e->op == "+" || e->op == "-");

  std::string lhs_reg = isAddition ? "%rbx" : "%rax";
  std::string rhs_reg = isAddition ? "%rcx" : "%rbx";

  visitExpr(e->lhs);
  visitExpr(e->rhs);

  // Pop the right hand side
  push(Instr{.var = PopInstr{.where = rhs_reg}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  // Pop the left hand side
  push(Instr{.var = PopInstr{.where = lhs_reg}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  // Perform the binary operation
  std::string op = lookup(opMap, e->op);
  if (op == "imul" || op == "idiv") {
    push(Instr{.var = BinaryInstr{.op = op, .src = rhs_reg},
               .type = InstrType::Binary},
         Section::Main);
  } else if (op == "add") {
    push(Instr{.var = AddInstr{.lhs = lhs_reg, .rhs = rhs_reg},
               .type = InstrType::Add},
         Section::Main);
  } else if (op == "sub") {
    push(Instr{.var = SubInstr{.lhs = lhs_reg, .rhs = rhs_reg},
               .type = InstrType::Sub},
         Section::Main);
  }

  // Push the result
  push(Instr{.var = PushInstr{.what = lhs_reg}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
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