#pragma once

#include "optimize.hpp"

#include <string>
#include <vector>

class Stringifier {
public:
  inline static std::string stringifyInstrs(std::vector<Instr> &input) {
    std::string output{};
    for (Instr &instr : input) {
      output += stringify(instr);
    }
    return output;
  }

  inline static std::string stringify(Instr instr) {
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
      std::string operator()(CmpInstr instr) const {
        return "cmp " + instr.lhs + ", " + instr.rhs + "\n\t";
      }
      std::string operator()(JumpInstr instr) const {
        std::string keyword = {};
        switch (instr.op) {
          case JumpCondition::Zero:
            keyword = "jz";
            break;
          case JumpCondition::NotZero:
            keyword = "jnz";
            break;
          case JumpCondition::Greater:
            keyword = "jg";
            break;
          case JumpCondition::GreaterEqual:
            keyword = "jge";
            break;
          case JumpCondition::NotGreater:
            keyword = "jng";
            break;
          case JumpCondition::Less:
            keyword = "jl";
            break;
          case JumpCondition::LessEqual:
            keyword = "jle";
            break;
          case JumpCondition::NotLess:
            keyword = "jnl";
            break;
          case JumpCondition::Equal:
            keyword = "je";
            break;
          case JumpCondition::NotEqual:
            keyword = "jne";
            break;
          
          case JumpCondition::Unconditioned:
            keyword = "jmp";
            break;
          default:
            keyword = "UNIMPLEMENTED";
            break;
        }
        return keyword + " " + instr.label + "\n\t";
      }
      std::string operator()(SetInstr instr) const {
        return "set" + instr.what + " " + instr.where + "\n\t";
      }
      std::string operator()(Syscall instr) const {
        return "syscall ; " + instr.name + "\n\t";
      }
      std::string operator()(Ret instr) const { return "ret\n\t"; }
      std::string operator()(Comment instr) const {
        return "; " + instr.comment + "\n\t";
      }
    };
    return std::visit(InstrVisitor {}, instr.var);
  }
};