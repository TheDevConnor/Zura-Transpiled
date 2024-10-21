#include "../helper/error/error.hpp"
#include "gen.hpp"
#include <fstream>

void codegen::handlerError(int line, int pos, std::string msg, std::string note,
                           std::string typeOfError) {
  Lexer lexer; // dummy lexer
  if (note != "")
    ErrorClass::error(line, pos, msg, note, typeOfError, node.current_file,
                      lexer, node.tks, false, false, false, false, false, true);
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, false, false, false, true);
}

void codegen::pushRegister(const std::string &reg) {
  push(Instr{.var=PushInstr{.what=reg},.type= InstrType::Push},
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

JumpCondition codegen::getJumpCondition(const std::string &op) {
  if (op == ">") return JumpCondition::Greater;
  if (op == ">=") return JumpCondition::GreaterEqual;
  if (op == "<") return JumpCondition::Less;
  if (op == "<=") return JumpCondition::LessEqual;
  if (op == "==") return JumpCondition::Equal;
  if (op == "!=") return JumpCondition::NotEqual;
  std::cerr << "Invalid operator for comparison" << std::endl;
  exit(-1);
}

void codegen::processBinaryExpression(BinaryExpr *cond, const std::string &preconCount, const std::string &name, bool isLoop) {
  // Evaluate LHS first, store in %rax
  visitExpr(cond->lhs);
  popToRegister("%rax");

  // Evaluate RHS, store in %rbx
  visitExpr(cond->rhs);
  popToRegister("%rbx");

  if (isLoop) {
    // Perform comparison (this order ensures LHS < RHS works correctly)(loops)
    push(Instr{.var = CmpInstr{.lhs = "%rbx", .rhs = "%rax"}, .type = InstrType::Cmp}, Section::Main);
  } else {
    // Perform comparison (this order ensures LHS > RHS works correctly)(ifs)
    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"}, .type = InstrType::Cmp}, Section::Main);
  }

  // Jump based on the correct condition
  JumpCondition jmpCond = getJumpCondition(cond->op);
  push(Instr{.var = JumpInstr{.op = jmpCond, .label = name + preconCount}, .type = InstrType::Jmp}, Section::Main);
}

void codegen::handleExitSyscall() {
  moveRegister("%rdi", "%rax", DataSize::Qword, DataSize::Qword);
  moveRegister("%rax", "$60", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_EXIT"}, .type = InstrType::Syscall}, Section::Main);
}

void codegen::handleReturnCleanup() {
  if (stackSize - funcBlockStart == 0) {
    push(Instr{.var = PopInstr{.where = "%rbp", .whereSize = DataSize::Qword}, .type = InstrType::Pop}, Section::Main);
    stackSize--;
    push(Instr{.var=LinkerDirective{.value=".cfi_def_cfa 7, 8\n\t"},.type=InstrType::Linker},Section::Main);
  } else {
    push(Instr{.var = MovInstr{.dest = "%rbp", .src = std::to_string(8 * (stackSize - funcBlockStart)) + "(%rsp)", .destSize = DataSize::Qword, .srcSize = DataSize::Qword}, .type = InstrType::Mov}, Section::Main);
  }
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
  if (log.is_open()) {
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
  }
  log.close();

  if (result != 0) {
    handlerError(0, 0, "Error executing command: " + command, log_contents,
                 "Codegen Error");
    return false;
  }
  return true;
}

void codegen::push(Instr instr, Section section) {
  if (section == Section::Main) {
    text_section.push_back(instr);
  } else if (section == Section::Head) {
    head_section.push_back(instr);
  } else if (section == Section::Data) {
    data_section.push_back(instr);
  } else if (section == Section::ReadonlyData) {
    rodt_section.push_back(instr);
  }
}