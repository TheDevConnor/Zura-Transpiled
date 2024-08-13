#include "gen.hpp"
#include "optimize.hpp"

void codegen::visitExpr(Node::Expr *expr) {
  auto handler = lookup(exprHandlers, expr->kind);
  if (handler) {
    handler(expr);
  }
}

void codegen::binary(Node::Expr *expr) { 
    auto binary = static_cast<BinaryExpr *>(expr); // segfau
    bool isAdditive = (binary->op[0] == '+' || binary->op[0] == '-');

    // ADD / MUL are different              ADD     MUL
    std::string registerLhs = isAdditive ? "rbx" : "rax";
    std::string registerRhs = isAdditive ? "rdx" : "rcx";
    visitExpr(binary->lhs);
    push(Instr { .var = PopInstr { .where = registerLhs }, .type = InstrType::Pop }, true);
    stackSize--;
    visitExpr(binary->rhs);
    push(Instr { .var = PopInstr { .where = registerRhs }, .type = InstrType::Pop }, true);
    stackSize--;

    switch (binary->op[0]) {
        case '+':
          // push(Optimezer::Instr { .var = Comment { .comment = "Addition" }, .type = InstrType::Comment });
          push(Instr { .var = AddInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Add }, true);
          break;
        case '-':
          // push(Optimezer::Instr { .var = Comment { .comment = "Subtraction" }, .type = InstrType::Comment });
          push(Instr { .var = SubInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Sub }, true);
          break;
        case '*':
          // push(Optimezer::Instr { .var = Comment { .comment = "Multiplication" }, .type = InstrType::Comment }, true);
          push(Instr { .var = MulInstr { .from = registerRhs }, .type = InstrType::Mul }, true);
          break;
        case '/':
          // push(Optimezer::Instr { .var = Comment { .comment = "Division" }, .type = InstrType::Comment })
          // rdx is the upper-64 bits of the first param, so make sure we don't divide by something stupid
          push(Instr { .var = XorInstr { .lhs = "rdx", .rhs = "rdx"}, .type = InstrType::Xor }, true);
          push(Instr { .var = DivInstr { .from = registerRhs }, .type = InstrType::Div }, true);
          break;
        default:
            break;
    }

    // Push result back onto stack (result typically in `rax` or `rbx`/`rcx`)
    push(Instr { .var = PushInstr { .what = registerLhs }, .type = InstrType::Push }, true);
    stackSize++;
}

void codegen::grouping(Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);

  visitExpr(grouping->expr);
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
    push(Instr { .var = PushInstr { .what = res }, .type = InstrType::Push }, true);
    stackSize++;
    break;
  }
  case ND_IDENT: {
    auto ident = static_cast<IdentExpr *>(expr);

    size_t offset = (stackSize - stackTable.at(ident->name)) * 8;
    // is this is it
    push(Instr { .var = Comment { .comment = "clone variable '" + ident->name + "'" } }, true);
    if (offset == 0) {
      push(Instr { .var = PushInstr { .what = "qword [rsp]" }, .type = InstrType::Push }, true);
      stackSize++;
    } else {
      push(Instr { .var = PushInstr { .what = "qword [rsp + " + std::to_string(offset) + "]" }, .type = InstrType::Push }, true);
      stackSize++;
    }
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    // push("\tpush " + string->value, true);
    // stackSize++;
    break;
  }
  case ND_BOOL: {
    auto boolean = static_cast<BoolExpr *>(expr);
    // push("\tpush " + std::to_string(boolean->value), true);
    // stackSize++;
    break;
  }
  default:
    break;
  }
}