#pragma once

#include "optimize.hpp"

#include <string>
#include <vector>

class Stringifier {
public:
  inline static std::string stringifyInstrs(std::vector<Optimezer::Instr> &input) {
    std::string output{};
    for (Optimezer::Instr &instr : input) {
      output += stringify(instr);
    }
    return output;
  }

  inline static std::string stringify(Optimezer::Instr instr) {
    struct InstrVisitor {
      std::string operator()(MovInstr instr) const {
        return "mov " + instr.dest + ", " + instr.src + "\n\t";
      }
      std::string operator()(PushInstr instr) const {
        return "push " + instr.what + "\n\t";
      }
      std::string operator()(PopInstr instr) const {
        return "pop " + instr.where + "\n\t";
      }
      std::string operator()(XorInstr instr) const {
        return "xor " + instr.lhs + ", " + instr.rhs + "\n\t";
      }
      std::string operator()(AddInstr instr) const {
        return "add " + instr.lhs + ", " + instr.rhs + "\n\t";
      }
      std::string operator()(SubInstr instr) const {
        return "sub " + instr.lhs + ", " + instr.rhs + "\n\t";
      }
      std::string operator()(MulInstr instr) const {
        return "mul " + instr.from + "\n\t";
      }
      std::string operator()(DivInstr instr) const {
        return "div " + instr.from + "\n\t";
      }
      std::string operator()(Label instr) const {
        return "\n" + instr.name + ":\n\t";
      }
      std::string operator()(Syscall instr) const {
        return "syscall ; " + instr.name + "\n\t";
      }
      std::string operator()(Ret instr) const { return "ret\n\t"; }
      std::string operator()(Comment instr) const {
        return "; " + instr.comment + "\n\t";
      }
    };
    InstrVisitor visitor = {};
    return std::visit(visitor, instr.var);
  }
};