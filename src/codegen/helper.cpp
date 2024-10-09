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
  }
}