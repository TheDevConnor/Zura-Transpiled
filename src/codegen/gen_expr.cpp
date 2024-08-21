#include "gen.hpp"
#include "optimize.hpp"
#include <cstddef>
#include <iostream>

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
  push(Instr{.var = PopInstr{.where = registerLhs}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;
  visitExpr(binary->rhs);
  push(Instr{.var = PopInstr{.where = registerRhs}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  switch (binary->op[0]) {
  case '+':
    // push(Optimezer::Instr { .var = Comment { .comment = "Addition" }, .type =
    // InstrType::Comment });
    push(Instr{.var = AddInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Add},
         Section::Main);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  case '-':
    // push(Optimezer::Instr { .var = Comment { .comment = "Subtraction" },
    // .type = InstrType::Comment });
    push(Instr{.var = SubInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Sub},
         Section::Main);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  case '*':
    // push(Optimezer::Instr { .var = Comment { .comment = "Multiplication" },
    // .type = InstrType::Comment }, true);
    push(Instr{.var = MulInstr{.from = registerRhs}, .type = InstrType::Mul},
         Section::Main);
    break;
  case '/':
    // push(Optimezer::Instr { .var = Comment { .comment = "Division" }, .type =
    // InstrType::Comment }) rdx is the upper-64 bits of the first param, so
    // make sure we don't divide by something stupid
    push(Instr{.var = XorInstr{.lhs = "rdx", .rhs = "rdx"},
               .type = InstrType::Xor},
         Section::Main);
    push(Instr{.var = DivInstr{.from = registerRhs}, .type = InstrType::Div},
         Section::Main);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  case '%':
    // push(Optimezer::Instr { .var = Comment { .comment = "Modulus" }, .type =
    // InstrType::Comment });
    push(Instr{.var = XorInstr{.lhs = "rdx", .rhs = "rdx"},
               .type = InstrType::Xor},
         Section::Main);
    push(Instr{.var = DivInstr{.from = registerRhs}, .type = InstrType::Div},
         Section::Main);
    push(Instr{.var = MovInstr{.dest = registerLhs, .src = "rdx"},
               .type = InstrType::Mov},
         Section::Main);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  // Comparisons
  case '>':
    // push(Optimezer::Instr { .var = Comment { .comment = "Greater Than" },
    // .type = InstrType::Comment });
    push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Cmp},
         Section::Main);
    if (binary->op[1] == '=') {
      push(Instr{.var = JumpInstr{.op = JumpCondition::GreaterEqual,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           Section::Main);
    } else {
      push(Instr{.var = JumpInstr{.op = JumpCondition::Greater,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           Section::Main);
    }
    pushCompAsExpr();
    break;
  case '<':
    // push(Optimezer::Instr { .var = Comment { .comment = "Less Than" }, .type
    // = InstrType::Comment });
    push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Cmp},
         Section::Main);
    if (binary->op[1] == '=') {
      push(Instr{.var = JumpInstr{.op = JumpCondition::LessEqual,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           Section::Main);
      pushCompAsExpr();
      break;
    }
    push(Instr{.var = JumpInstr{.op = JumpCondition::Less,
                                .label = "conditional" +
                                         std::to_string(++conditionalCount)},
               .type = InstrType::Jmp},
         Section::Main);
    pushCompAsExpr();
    break;
  case '=':
    if (binary->op[1] == '=') {
      push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
                 .type = InstrType::Cmp},
           Section::Main);
      push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           Section::Main);
      pushCompAsExpr();
      break;
    }

    // if its just one equal sign, then it is a assign expr
    break;
  case '!':
    if (binary->op[1] == '=') {
      // push(Optimezer::Instr { .var = Comment { .comment = "Not Equal" },
      // .type = InstrType::Comment });
      push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
                 .type = InstrType::Cmp},
           Section::Main);
      push(Instr{.var = JumpInstr{.op = JumpCondition::NotEqual,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           Section::Main);
      pushCompAsExpr();
      break;
    }

    // unary NOT operation (visited elsewhere)
    break;
  default:
    break;
  }
}

void codegen::pushCompAsExpr() {
  std::string preConditionalCount = std::to_string(conditionalCount);

  // assume condition failed (we would have jumped past here if we didn't)
  push(Instr{.var = PushInstr{.what = "0x0"}, .type = InstrType::Push},
       Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       Section::Main);

  push(Instr{.var = Label{.name =
                              "conditional" + std::to_string(conditionalCount)},
             .type = InstrType::Label},
       Section::Main);

  push(Instr{.var = PushInstr{.what = "0x1"}, .type = InstrType::Push},
       Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       Section::Main);

  push(Instr{.var = Label{.name = "main" + preConditionalCount},
             .type = InstrType::Label},
       Section::Main);
  stackSize += 2;
  conditionalCount++;
  return;
}

void codegen::grouping(Node::Expr *expr) {
  GroupExpr *grouping = static_cast<GroupExpr *>(expr);

  visitExpr(grouping->expr);
}

void codegen::unary(Node::Expr *expr) {
  auto unary = static_cast<UnaryExpr *>(expr);

  visitExpr(unary->expr);
  switch (unary->op[0]) {
  case '-': // !NOTE: added in negatives but they are not working properly
    push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    push(Instr{.var = NegInstr{.what = "rax"}, .type = InstrType::Neg},
         Section::Main);
    break;
  case '!': // !NOTE: same as above
    push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    push(Instr{.var = NotInstr{.what = "rax"}, .type = InstrType::Not},
         Section::Main);
    break;
  case '+':
    if (unary->op[1] == '+') {
      // Pop the current value into rax
      push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop},
           Section::Main);
      stackSize--;

      // Increment rax by 1
      push(Instr{.var = AddInstr{.lhs = "rax", .rhs = "1"},
                 .type = InstrType::Add},
           Section::Main);

      // Push the incremented value back onto the stack
      push(Instr{.var = PushInstr{.what = "rax"}, .type = InstrType::Push},
           Section::Main);
      stackSize++;

      break;
    }
  default:
    break;
  }

  push(Instr{.var = PushInstr{.what = "rax"}, .type = InstrType::Push},
       Section::Main);
}

void codegen::call(Node::Expr *expr) {
  auto call = static_cast<CallExpr *>(expr);

  for (auto arg : call->args) {
    visitExpr(arg);
  }

  std::string callee = static_cast<IdentExpr *>(call->callee)->name;

  push(Instr{.var = Comment{.comment = "Call function '" + callee + "'"},
             .type = InstrType::Comment},
       Section::Main);
  push(Instr{.var = CallInstr{.name = callee}, .type = InstrType::Call},
       Section::Main);
  stackSize -= call->args.size();
  push(Instr{.var = PushInstr{.what = "rax"}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

void codegen::ternary(Node::Expr *expr) {
  auto ternary = static_cast<TernaryExpr *>(expr);
  push(Instr{.var = Comment{.comment = "Ternary operation"},
             .type = InstrType::Comment},
       Section::Main);

  std::string ternayCount = std::to_string(++conditionalCount);

  visitExpr(ternary->condition);
  push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  push(Instr{.var = CmpInstr{.lhs = "rax", .rhs = "0"}, .type = InstrType::Cmp},
       Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                              .label = "ternaryFalse" + ternayCount},
             .type = InstrType::Jmp},
       Section::Main);

  visitExpr(ternary->lhs);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "ternaryEnd" + ternayCount},
             .type = InstrType::Jmp},
       Section::Main);
  push(Instr{.var = Label{.name = "ternaryFalse" + ternayCount},
             .type = InstrType::Label},
       Section::Main);

  visitExpr(ternary->rhs);

  push(Instr{.var = Label{.name = "ternaryEnd" + ternayCount},
             .type = InstrType::Label},
       Section::Main);

  stackSize++;
}

void codegen::primary(Node::Expr *expr) {
  switch (expr->kind) {
  case ND_NUMBER: {
    auto number = static_cast<NumberExpr *>(expr);
    auto res = (number->value == (int)number->value)
                   ? std::to_string((int)number->value)
                   : std::to_string(number->value);
    push(Instr{.var = PushInstr{.what = res}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  }
  case ND_IDENT: {
    auto ident = static_cast<IdentExpr *>(expr);

    size_t offset = (stackSize - stackTable.at(ident->name)) * 8;
    push(Instr{.var =
                   Comment{.comment = "clone variable '" + ident->name + "'"}},
         Section::Main);
    // dont bother doing "qword [rsp + 0]"
    if (offset == 0) {
      push(Instr{.var = PushInstr{.what = "qword [rsp]"},
                 .type = InstrType::Push},
           Section::Main);
      stackSize++;
    } else {
      push(Instr{.var = PushInstr{.what = "qword [rsp + " +
                                          std::to_string(offset) + "]"},
                 .type = InstrType::Push},
           Section::Main);
      stackSize++;
    }
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    std::string label = "string" + std::to_string(stringCount++);

    // Push the label onto the stack
    push(Instr{.var = PushInstr{.what = label}, .type = InstrType::Push},
         Section::Main);
    stackSize++;

    // Define the label for the string
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::Data);

    // Store the processed string
    push(Instr{.var = DBInstr{.what = string->value + ", 00"},
               .type = InstrType::DB}, // add NUL terminator
         Section::Data);

    break;
  }
  case ND_BOOL: {
    auto boolean = static_cast<BoolExpr *>(expr);
    push(Instr{.var = PushInstr{.what = boolean->value ? "1" : "0"},
               .type = InstrType::Push},
         Section::Main);
    stackSize++;
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
    std::cerr << "Variable '" + assignee->name +
                     "' not predefined, cannot reassign"
              << std::endl;
    exit(EXIT_FAILURE); // c macro moment
  }
  visitExpr(assignExpr->rhs);
  int offset = (stackSize - stackTable.at(assignee->name)) - 1;

  if (offset == 0) {
    // dont bother doing [ + 0]
    push(Instr{.var = PopInstr{.where = "qword [rsp]"}, .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    return;
  }
  push(Instr{.var = PopInstr{.where = "qword [rsp + " +
                                      std::to_string(offset * 8) + "]"},
             .type = InstrType::Pop},
       Section::Main);
  stackSize--;
}