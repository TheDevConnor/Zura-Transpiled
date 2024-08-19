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
  push(Instr{.var = PopInstr{.where = registerLhs}, .type = InstrType::Pop},
       true);
  stackSize--;
  visitExpr(binary->rhs);
  push(Instr{.var = PopInstr{.where = registerRhs}, .type = InstrType::Pop},
       true);
  stackSize--;

  switch (binary->op[0]) {
  case '+':
    // push(Optimezer::Instr { .var = Comment { .comment = "Addition" }, .type =
    // InstrType::Comment });
    push(Instr{.var = AddInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Add},
         true);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         true);
    stackSize++;
    break;
  case '-':
    // push(Optimezer::Instr { .var = Comment { .comment = "Subtraction" },
    // .type = InstrType::Comment });
    push(Instr{.var = SubInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Sub},
         true);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         true);
    stackSize++;
    break;
  case '*':
    // push(Optimezer::Instr { .var = Comment { .comment = "Multiplication" },
    // .type = InstrType::Comment }, true);
    push(Instr{.var = MulInstr{.from = registerRhs}, .type = InstrType::Mul},
         true);
    break;
  case '/':
    // push(Optimezer::Instr { .var = Comment { .comment = "Division" }, .type =
    // InstrType::Comment }) rdx is the upper-64 bits of the first param, so
    // make sure we don't divide by something stupid
    push(Instr{.var = XorInstr{.lhs = "rdx", .rhs = "rdx"},
               .type = InstrType::Xor},
         true);
    push(Instr{.var = DivInstr{.from = registerRhs}, .type = InstrType::Div},
         true);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         true);
    stackSize++;
    break;
  case '%':
    // push(Optimezer::Instr { .var = Comment { .comment = "Modulus" }, .type =
    // InstrType::Comment });
    push(Instr{.var = XorInstr{.lhs = "rdx", .rhs = "rdx"},
               .type = InstrType::Xor},
         true);
    push(Instr{.var = DivInstr{.from = registerRhs}, .type = InstrType::Div},
         true);
    push(Instr{.var = MovInstr{.dest = registerLhs, .src = "rdx"},
               .type = InstrType::Mov},
         true);
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         true);
    stackSize++;
    break;
  // Comparisons
  case '>':
    // push(Optimezer::Instr { .var = Comment { .comment = "Greater Than" },
    // .type = InstrType::Comment });
    push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Cmp},
         true);
    if (binary->op[1] == '=') {
      push(Instr{.var = JumpInstr{.op = JumpCondition::GreaterEqual,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           true);
    } else {
      push(Instr{.var = JumpInstr{.op = JumpCondition::Greater,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           true);
    }
    pushCompAsExpr();
    break;
  case '<':
    // push(Optimezer::Instr { .var = Comment { .comment = "Less Than" }, .type
    // = InstrType::Comment });
    push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
               .type = InstrType::Cmp},
         true);
    if (binary->op[1] == '=') {
      push(Instr{.var = JumpInstr{.op = JumpCondition::LessEqual,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           true);
      pushCompAsExpr();
      break;
    }
    push(Instr{.var = JumpInstr{.op = JumpCondition::Less,
                                .label = "conditional" +
                                         std::to_string(++conditionalCount)},
               .type = InstrType::Jmp},
         true);
    pushCompAsExpr();
    break;
  case '=':
    if (binary->op[1] == '=') {
      push(Instr{.var = CmpInstr{.lhs = registerLhs, .rhs = registerRhs},
                 .type = InstrType::Cmp},
           true);
      push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           true);
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
           true);
      push(Instr{.var = JumpInstr{.op = JumpCondition::NotEqual,
                                  .label = "conditional" +
                                           std::to_string(++conditionalCount)},
                 .type = InstrType::Jmp},
           true);
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
  push(Instr{.var = PushInstr{.what = "0x0"}, .type = InstrType::Push}, true);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       true);

  push(Instr{.var = Label{.name =
                              "conditional" + std::to_string(conditionalCount)},
             .type = InstrType::Label},
       true);

  push(Instr{.var = PushInstr{.what = "0x1"}, .type = InstrType::Push}, true);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       true);

  push(Instr{.var = Label{.name = "main" + preConditionalCount},
             .type = InstrType::Label},
       true);
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
    push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop}, true);
    stackSize--;
    push(Instr{.var = NegInstr{.what = "rax"}, .type = InstrType::Neg}, true);
    break;
  case '!': // !NOTE: same as above
    push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop}, true);
    stackSize--;
    push(Instr{.var = NotInstr{.what = "rax"}, .type = InstrType::Not}, true);
    break;
  default:
    break;
  }

  push(Instr{.var = PushInstr{.what = "rax"}, .type = InstrType::Push}, true);
}

void codegen::call(Node::Expr *expr) {
  auto call = static_cast<CallExpr *>(expr);

  for (auto arg : call->args) {
    visitExpr(arg);
  }

  std::string callee = static_cast<IdentExpr *>(call->callee)->name;

  push(Instr{.var = Comment{.comment = "Call function '" + callee + "'"},
             .type = InstrType::Comment},
       true);
  push(Instr{.var = CallInstr{.name = "user_" + callee}, .type = InstrType::Call}, true);
  stackSize -= call->args.size();
  push(Instr{.var = PushInstr{.what = "rax"}, .type = InstrType::Push}, true);
  stackSize++;
}

void codegen::ternary(Node::Expr *expr) {
  auto ternary = static_cast<TernaryExpr *>(expr);
  push(Instr{.var = Comment{.comment = "Ternary operation"},
             .type = InstrType::Comment},
       true);

  std::string ternayCount = std::to_string(++conditionalCount);

  visitExpr(ternary->condition);
  push(Instr{.var = PopInstr{.where = "rax"}, .type = InstrType::Pop}, true);
  stackSize--;

  push(Instr{.var = CmpInstr{.lhs = "rax", .rhs = "0"}, .type = InstrType::Cmp},
       true);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                              .label = "ternaryFalse" + ternayCount},
             .type = InstrType::Jmp},
       true);

  visitExpr(ternary->lhs);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "ternaryEnd" + ternayCount},
             .type = InstrType::Jmp},
       true);
  push(Instr{.var = Label{.name = "ternaryFalse" + ternayCount},
             .type = InstrType::Label},
       true);

  visitExpr(ternary->rhs);

  push(Instr{.var = Label{.name = "ternaryEnd" + ternayCount},
             .type = InstrType::Label},
       true);

  stackSize++;
}

void codegen::primary(Node::Expr *expr) {
  switch (expr->kind) {
  case ND_NUMBER: {
    auto number = static_cast<NumberExpr *>(expr);
    auto res = (number->value == (int)number->value)
                   ? std::to_string((int)number->value)
                   : std::to_string(number->value);
    push(Instr{.var = PushInstr{.what = res}, .type = InstrType::Push}, true);
    stackSize++;
    break;
  }
  case ND_IDENT: {
    auto ident = static_cast<IdentExpr *>(expr);

    size_t offset = (stackSize - stackTable.at(ident->name)) * 8;
    // is this is it
    push(Instr{.var =
                   Comment{.comment = "clone variable '" + ident->name + "'"}},
         true);
    if (offset == 0) {
      push(Instr{.var = PushInstr{.what = "qword [rsp]"},
                 .type = InstrType::Push},
           true);
      stackSize++;
    } else {
      push(Instr{.var = PushInstr{.what = "qword [rsp + " +
                                          std::to_string(offset) + "]"},
                 .type = InstrType::Push},
           true);
      stackSize++;
    }
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    std::string label = "string" + std::to_string(stringCount++);

    // Push the label onto the stack
    push(Instr{.var = PushInstr{.what = label}, .type = InstrType::Push}, true);
    stackSize++;

    // Define the label for the string
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label}, false);

    // Store the processed string
    push(Instr{.var = DBInstr{.what = string->value}, .type = InstrType::DB},
         false);

    break;
  }
  case ND_BOOL: {
    auto boolean = static_cast<BoolExpr *>(expr);
    push(Instr{.var = PushInstr{.what = boolean->value ? "1" : "0"},
               .type = InstrType::Push},
         true);
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
         true);
    stackSize--;
    return;
  }
  push(Instr{.var = PopInstr{.where = "qword [rsp + " +
                                      std::to_string(offset * 8) + "]"},
             .type = InstrType::Pop},
       true);
  stackSize--;
}