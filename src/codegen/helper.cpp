#include "../helper/error/error.hpp"
#include "gen.hpp"
#include "optimizer/instr.hpp"
#include <fstream>

void codegen::handlerError(int line, int pos, std::string msg,
                           std::string typeOfError) {
  Lexer lexer; // dummy lexer
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, false, 
                    false, false, true);
}

void codegen::pushRegister(const std::string &reg) {
  push(Instr{.var = PushInstr{.what = reg}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

void codegen::popToRegister(const std::string &reg) {
  push(Instr{.var = PopInstr({.where = reg}), .type = InstrType::Pop},
       Section::Main);
  stackSize--;
}

void codegen::moveRegister(const std::string &dest, const std::string &src,
                           DataSize dest_size, DataSize src_size) {
  push(Instr{.var = MovInstr{.dest = dest,
                             .src = src,
                             .destSize = dest_size,
                             .srcSize = src_size},
             .type = InstrType::Mov},
       Section::Main);
}

void codegen::pushLinker(std::string val, Section section) {
  push(Instr {
    .var = LinkerDirective {
      .value = val
    }
  }, section);
}

JumpCondition codegen::getJumpCondition(const std::string &op) {
  if (op == ">")
    return JumpCondition::Greater;
  if (op == ">=")
    return JumpCondition::GreaterEqual;
  if (op == "<")
    return JumpCondition::Less;
  if (op == "<=")
    return JumpCondition::LessEqual;
  if (op == "==")
    return JumpCondition::Equal;
  if (op == "!=")
    return JumpCondition::NotEqual;
  std::cerr << "Invalid operator for comparison" << std::endl;
  exit(-1);
}

int codegen::getExpressionDepth(BinaryExpr *e) {
    int depth = 0;
    // We don't need to check for the depth of the left-hand side because it's already been visited
    if (e->rhs->kind == NodeKind::ND_BINARY) {
        int rhsDepth = getExpressionDepth(static_cast<BinaryExpr *>(e->rhs));
        if (rhsDepth > depth) {
            depth = rhsDepth;
        }
    }
    return depth + 1;
}

void codegen::processBinaryExpression(BinaryExpr *cond,
                                      const std::string &preconCount,
                                      const std::string &name, bool isLoop) {
  // TODO: Float comparisons
  // Evaluate LHS first, store in %rax
  visitExpr(cond->lhs);
  popToRegister("%rax");

  // Evaluate RHS, store in %rbx
  visitExpr(cond->rhs);
  popToRegister("%rbx");

  if (isLoop) {
    // Perform comparison (this order ensures LHS > RHS works correctly)(loops)
    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"},
               .type = InstrType::Cmp},
         Section::Main);

    // Jump based on the correct condition
    JumpCondition jmpCond = getJumpCondition(cond->op);

    // Recerse the condition for loops (e.g., for i = 0; i < 10; i++) to jump if i >= 10
    switch (jmpCond) {
      case JumpCondition::Less:
        jmpCond = JumpCondition::GreaterEqual;
        break;
      case JumpCondition::LessEqual:
        jmpCond = JumpCondition::Greater;
        break;
      case JumpCondition::Greater:
        jmpCond = JumpCondition::LessEqual;
        break;
      case JumpCondition::GreaterEqual:
        jmpCond = JumpCondition::Less;
        break;
      default:
        break;
      }

    push(Instr{.var = JumpInstr{.op = jmpCond, .label = name },
               .type = InstrType::Jmp},
         Section::Main);
    return;
  } else {
    // Perform comparison (this order ensures LHS > RHS works correctly)(ifs)
    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"},
               .type = InstrType::Cmp},
         Section::Main);
  }

  // Jump based on the correct condition
  JumpCondition jmpCond = getJumpCondition(cond->op);
  push(Instr{.var = JumpInstr{.op = jmpCond, .label = name + preconCount},
             .type = InstrType::Jmp},
       Section::Main);
}

void codegen::handleExitSyscall() {
  moveRegister("%rdi", "%rax", DataSize::Qword, DataSize::Qword);
  moveRegister("%rax", "$60", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_EXIT"}, .type = InstrType::Syscall},
       Section::Main);
}

void codegen::handleReturnCleanup() {
  popToRegister("%rbp");
}

int codegen::convertFloatToInt(float input) {
  union {
    float f;
    int i;
  } u;
  u.f = input;
  return u.i;
}

bool codegen::execute_command(const std::string &command,
                              const std::string &log_file) {
  std::string command_with_error_logging = command + " 2> " + log_file;
  int result = std::system(command_with_error_logging.c_str());
  // Read log file
  std::ifstream log(log_file);
  std::string log_contents = "";

  if (!log.is_open()) {
    handlerError(0, 0, "Error opening log file: " + log_file, "Codegen Error");
    return false;
  }
  
  std::string line;
  bool first = true;
  while (getline(log, line)) {
    if (first) {
      log_contents += line;
      first = false;
    } else {
      log_contents += "\n\t" + line;
    }
  }
  
  log.close();

  if (result != 0) {
    handlerError(0, 0,
                 "Error executing command: " + command + "\n\t" + log_contents,
                 "Codegen Error");
    return false;
  }
  return true;
}

void codegen::push(Instr instr, Section section) {
  switch (section) {
    case Section::Main:
      text_section.push_back(instr);
      break;
    case Section::Head:
      head_section.push_back(instr);
      break;
    case Section::Data:
      data_section.push_back(instr);
      break;
    case Section::ReadonlyData:
      rodt_section.push_back(instr);
      break;
    case Section::DIE:
      die_section.push_back(instr);
      break;
    case Section::DIEString:
      dies_section.push_back(instr);
      break;
    case Section::DIEAbbrev:
      diea_section.push_back(instr);
      break;
  }
}

void codegen::pushDebug(int line) {
  if (debug)
    push(Instr{.var = LinkerDirective{.value = ".loc 0 " +
                                               std::to_string(line) + "\n\t"},
               .type = InstrType::Linker},
         Section::Main);
}