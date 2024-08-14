#include "gen.hpp"
#include "optimize.hpp"
#include <cstddef>

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
          push(Instr { .var = PushInstr { .what = registerLhs }, .type = InstrType::Push }, true);
          stackSize++;
          break;
        case '-':
          // push(Optimezer::Instr { .var = Comment { .comment = "Subtraction" }, .type = InstrType::Comment });
          push(Instr { .var = SubInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Sub }, true);
          push(Instr { .var = PushInstr { .what = registerLhs }, .type = InstrType::Push }, true);
          stackSize++;
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
          push(Instr { .var = PushInstr { .what = registerLhs }, .type = InstrType::Push }, true);
          stackSize++;
          break;
        case '%':
          // push(Optimezer::Instr { .var = Comment { .comment = "Modulus" }, .type = InstrType::Comment });
          push(Instr { .var = XorInstr { .lhs = "rdx", .rhs = "rdx"}, .type = InstrType::Xor }, true);
          push(Instr { .var = DivInstr { .from = registerRhs }, .type = InstrType::Div }, true);
          push(Instr { .var = MovInstr { .dest = registerLhs, .src = "rdx" }, .type = InstrType::Mov }, true);
          push(Instr { .var = PushInstr { .what = registerLhs }, .type = InstrType::Push }, true);
          stackSize++;
          break;
        // Comparisons
        case '>':
          // push(Optimezer::Instr { .var = Comment { .comment = "Greater Than" }, .type = InstrType::Comment });
          push(Instr { .var = CmpInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Cmp }, true);
          if (binary->op[1] == '=') {
            push(Instr { .var = JumpInstr { .op = JumpCondition::GreaterEqual, .label = "conditional" + std::to_string(conditionalCount++) }, .type = InstrType::Jmp }, true);
            break;
          }
          push(Instr { .var = JumpInstr { .op = JumpCondition::Greater, .label = "conditional" + std::to_string(conditionalCount++) }, .type = InstrType::Jmp }, true);
          break;
        case '<':
          // push(Optimezer::Instr { .var = Comment { .comment = "Less Than" }, .type = InstrType::Comment });
          push(Instr { .var = CmpInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Cmp }, true);
          if (binary->op[1] == '=') {
            push(Instr { .var = JumpInstr { .op = JumpCondition::LessEqual, .label = "conditional" + std::to_string(conditionalCount++) }, .type = InstrType::Jmp}, true);
            break;
          }
          push(Instr { .var = JumpInstr { .op = JumpCondition::Less, .label = "conditional" + std::to_string(conditionalCount++) }, .type = InstrType::Jmp}, true);
          break;
        case '=':
          push(Instr { .var = CmpInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Cmp }, true);
          if (binary->op[1] == '=') {
            push (Instr { .var = JumpInstr { .op = JumpCondition::Equal, .label = "conditional" + std::to_string(conditionalCount++) }, .type = InstrType::Jmp }, true);
            break;
          }
          // idk, sounds like a redecl thing but too late to impl rn
          break;
        case '!':
          // push(Optimezer::Instr { .var = Comment { .comment = "Not Equal" }, .type = InstrType::Comment });
          push(Instr { .var = CmpInstr { .lhs = registerLhs, .rhs = registerRhs }, .type = InstrType::Cmp }, true);
          if (binary->op[1] == '=') {
            push (Instr { .var = JumpInstr { .op = JumpCondition::NotEqual, .label = "conditional" + std::to_string(conditionalCount++) }, .type = InstrType::Jmp }, true);
            break;
          }
          // unary NOT operation (visited elsewhere)
          break;
        default:
            break;
    }
}

void codegen::grouping(Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);

  visitExpr(grouping->expr);
}

void codegen::unary(Node::Expr *expr) {
  auto unary = static_cast<UnaryExpr *>(expr);

  visitExpr(unary->expr);
  switch (unary->op[0]) {
    case '-': // TODO: use x86 NEG instr
      push(Instr { .var = PopInstr { .where = "rbx" }, .type = InstrType::Pop }, true);
      stackSize--;
      push(Instr { .var = XorInstr { .lhs = "rax", .rhs = "rax" }, .type = InstrType::Xor }, true);
      push(Instr { .var = SubInstr { .lhs = "rax", .rhs = "rbx" }, .type = InstrType::Sub }, true);
      break;
    case '!':
      // TODO: use proper x86 NOT instr
      push(Instr { .var = PopInstr { .where = "rax" }, .type = InstrType::Pop }, true);
      stackSize--;
      push(Instr { .var = XorInstr { .lhs = "rax", .rhs = "rax" }, .type = InstrType::Xor }, true);
      push(Instr { .var = SetInstr { .where = "al" }, .type = InstrType::Set }, true);
      push(Instr { .var = MovInstr { .dest = "rax", .src = "0" }, .type = InstrType::Mov }, true);
      break;
    default:
      break;
  }
}

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

void codegen::assign(Node::Expr *expr) {
  auto assignExpr = static_cast<AssignmentExpr *>(expr);
  
  auto assignee = static_cast<IdentExpr *>(assignExpr->assignee);

  if (stackTable.empty()) {
    // how do you handle errors?
    std::cerr << "Variable '" + assignee->name + "' not predefined, cannot reassign" << std::endl;
    exit(EXIT_FAILURE); // c macro moment  
  }
  visitExpr(assignExpr->rhs);
  int offset = (stackSize - stackTable.at(assignee->name)) - 1;

  if (offset == 0) {
    // dont bother doing [ + 0]
    push(Instr { .var = PopInstr { .where = "qword [rsp]" }, .type = InstrType::Pop }, true);
    stackSize--;
    return;
  }
  push(Instr { .var = PopInstr { .where = "qword [rsp + " + std::to_string(offset * 8) + "]" }, .type = InstrType::Pop }, true);
  stackSize--;
}