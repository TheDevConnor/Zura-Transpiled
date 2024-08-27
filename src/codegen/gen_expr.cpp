#include "../helper/error/error.hpp"
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

void codegen::_arrayExpr(Node::Expr *expr) {
  ArrayExpr *arr = static_cast<ArrayExpr *>(expr);

  int elementCount = arr->elements.size();
  int elementsAlreadyPushed = 0;
  for (int i = 0; i < arrayCounts.size(); i++) {
    elementsAlreadyPushed += arrayCounts.at(i).second;
  }
  if (elementCount > 0) {
    for (int i = 0; i < elementCount; i++) {
      // Evaluate the argument
      visitExpr(arr->elements.at(i));
      push(Instr{.var = PopInstr{.where = "-" +
                                          std::to_string(
                                              8 * ((i + 1) +
                                                   elementsAlreadyPushed)) +
                                          "(%rbp)",
                                 .whereSize = DataSize::Qword},
                 .type = InstrType::Pop},
           Section::Main);
      stackSize--;
    }
  }
  arrayCounts.push_back(std::pair<size_t, size_t>(arrayCount++, elementCount));
  // Array is initialized!
  // vvv This is important later for when we access the items.
  push(Instr{.var =
                 PushInstr{.what = "$" + std::to_string(elementsAlreadyPushed),
                           .whatSize = DataSize::Qword},
             .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

// TODO: !!!! SUPER IMPORTANT: MAKE THIS ACTUALLY WORK LMAO!!!!!!!!
void codegen::arrayElem(Node::Expr *expr) {
  IndexExpr *realExpr = static_cast<IndexExpr *>(expr);
  visitExpr(realExpr->lhs);
  push(Instr{.var = PopInstr{.where = "%rdx", .whereSize = DataSize::Qword},
             .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  // Evalute rhs (the index -> arr[idx])
  visitExpr(realExpr->rhs);
  push(Instr{.var = PopInstr{.where = "%rax", .whereSize = DataSize::Qword},
             .type = InstrType::Pop},
       Section::Main);
  stackSize--;
  push(Instr{.var = AddInstr{.lhs = "%rdx", .rhs = "%rax"},
             .type = InstrType::Add},
       Section::Main);
  push(Instr{.var = AddInstr{.lhs = "%rdx", .rhs = "$1"},
             .type = InstrType::Add},
       Section::Main);
  push(Instr{.var = LinkerDirective{.value = "neg %rdx\n\t"},
             .type = InstrType::Linker},
       Section::Main);

  // Do not account for stack size. It doesn't matter because stackSize is rsp
  // and not rbp.
  push(Instr{.var = PushInstr{.what = "0(%rbp, %rdx, 8)",
                              .whatSize = DataSize::Qword},
             .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

void codegen::binary(Node::Expr *expr) { // kk
  auto binary = static_cast<BinaryExpr *>(expr);
  bool isAdditive = (binary->op[0] == '+' || binary->op[0] == '-');

  // ADD / MUL are different          ADDITIVE     MULTIPLICATIVE
  std::string registerLhs = isAdditive ? "%rbx" : "%rax";
  std::string registerRhs = isAdditive ? "%rdx" : "%rcx";
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
    push(Instr{.var = PushInstr{.what = registerLhs}, .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  case '/':
    // push(Optimezer::Instr { .var = Comment { .comment = "Division" }, .type =
    // InstrType::Comment }) rdx is the upper-64 bits of the first param, so
    // make sure we don't divide by something stupid
    push(Instr{.var = XorInstr{.lhs = "%rdx", .rhs = "%rdx"},
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
    push(Instr{.var = XorInstr{.lhs = "%rdx", .rhs = "%rdx"},
               .type = InstrType::Xor},
         Section::Main);
    push(Instr{.var = DivInstr{.from = registerRhs}, .type = InstrType::Div},
         Section::Main);
    push(Instr{.var = MovInstr{.dest = registerLhs, .src = "%rdx"},
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
  push(Instr{.var = PushInstr{.what = "$0x0"}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       Section::Main);

  push(Instr{.var = Label{.name =
                              "conditional" + std::to_string(conditionalCount)},
             .type = InstrType::Label},
       Section::Main);

  push(Instr{.var = PushInstr{.what = "$0x1"}, .type = InstrType::Push},
       Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       Section::Main);

  push(Instr{.var = Label{.name = "main" + preConditionalCount},
             .type = InstrType::Label},
       Section::Main);
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
  case '-': // !NOTE: signed numbers while the rest of math system is unsigned
    push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    push(Instr{.var = NegInstr{.what = "%rax"}, .type = InstrType::Neg},
         Section::Main);
    break;
  case '!': // !NOTE: same as above
    push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    push(Instr{.var = NotInstr{.what = "%rax"}, .type = InstrType::Not},
         Section::Main);
    break;
  case '+':
    if (unary->op[1] == '+') {
      // TODO: Implement the "inc" instruction for this very purpose
      // Pop the current value into rax
      push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop},
           Section::Main);
      stackSize--;

      // Increment rax by 1
      push(Instr{.var = AddInstr{.lhs = "%rax", .rhs = "$0x1"},
                 .type = InstrType::Add},
           Section::Main);

      // Push the incremented value back onto the stack
      push(Instr{.var = PushInstr{.what = "%rax"}, .type = InstrType::Push},
           Section::Main);
      stackSize++;

      break;
    }
  default:
    break;
  }

  push(Instr{.var = PushInstr{.what = "%rax"}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
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
  push(
      Instr{.var = CallInstr{.name = "usr_" + callee}, .type = InstrType::Call},
      Section::Main);
  push(Instr{.var = PushInstr{.what = "%rax"}, .type = InstrType::Push},
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
  push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;

  push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "$0"},
             .type = InstrType::Cmp},
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
  // TODO: Fix this stack issue
  visitExpr(ternary->rhs);

  push(Instr{.var = Label{.name = "ternaryEnd" + ternayCount},
             .type = InstrType::Label},
       Section::Main);
}

void codegen::primary(Node::Expr *expr) {
  switch (expr->kind) {
  case ND_NUMBER: {
    auto number = static_cast<NumberExpr *>(expr);
    auto res = (number->value == (int)number->value)
                   ? std::to_string((int)number->value)
                   : std::to_string(number->value);
    push(Instr{.var = PushInstr{.what = '$' + res, .whatSize = DataSize::Qword},
               .type = InstrType::Push},
         Section::Main);
    stackSize++;
    break;
  }
  case ND_IDENT: {
    auto ident = static_cast<IdentExpr *>(expr);
    int offset = (stackSize - stackTable.at(ident->name)) * 8;
    push(Instr{.var =
                   Comment{.comment = "clone variable '" + ident->name + "'"}},
         Section::Main);
    if (offset != 0) {
      push(Instr{.var = PushInstr{.what = std::to_string(offset) + "(%rsp)",
                                  .whatSize = DataSize::Qword},
                 .type = InstrType::Push},
           Section::Main);
      stackSize++;
      break;
    }
    push(Instr{.var = PushInstr{.what = "(%rsp)", .whatSize = DataSize::Qword},
               .type = InstrType::Push},
         Section::Main);

    stackSize++;
    break;
  }
  case ND_STRING: {
    auto string = static_cast<StringExpr *>(expr);
    std::string label = "string" + std::to_string(stringCount++);

    // Push the label onto the stack
    push(Instr{.var =
                   PushInstr{.what = '$' + label, .whatSize = DataSize::Qword},
               .type = InstrType::Push},
         Section::Main);
    stackSize++;

    // Define the label for the string
    push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
         Section::Data);

    // Store the processed string
    push(Instr{.var = DBInstr{.what = string->value + ", 00"},
               .type = InstrType::Asciz}, // Asciz should automatically add a 00
                                          // but we dont talk about that!
         Section::Data);

    break;
  }
  case ND_BOOL: {
    auto boolean = static_cast<BoolExpr *>(expr);
    push(Instr{.var = PushInstr{.what = "$" + std::string(1, boolean->value ? '1' : '0')},
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
	Lexer lexer;
    ErrorClass::error(assignee->line, assignee->pos,
                      "Variable '" + assignee->name +
                          "' not predefined, cannot reassign",
                      "", "Codegen Error", file_name, lexer, {}, false,
                      false, true, false, false, true);
  }
  visitExpr(assignExpr->rhs);
  int offset = (stackSize - stackTable.at(assignee->name)) - 1;

  if (offset == 0) {
    push(Instr{.var = PopInstr{.where = "(%rsp)", .whereSize = DataSize::Qword},
               .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    return;
  }
  push(Instr{.var = PopInstr{.where = std::to_string(offset * 8) + "(rsp)"},
             .type = InstrType::Pop},
       Section::Main);
  stackSize--;
}