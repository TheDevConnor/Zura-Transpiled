#pragma once

#include "optimize.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class Stringifier { // converts Instr structures into AT&T Syntax strings
public:
  inline static std::string stringifyInstrs(std::vector<Instr> &input) {
    std::string output{};
    for (Instr &instr : input) {
      output += stringify(instr);
    }
    return output;
  }

  inline static char dsToChar(DataSize in) {
    /* good to have for a quick look
    NONE,  0
    BYTE,  1
    WORD,  2
    DWORD, 3
    QWORD, 4 
    */
    switch (in) {
      case DataSize::Byte:
        return 'b';
      case DataSize::Word:
        return 'w';
      case DataSize::Dword:
        return 'l'; // 'long'
      case DataSize::Qword:
        return 'q';
      case DataSize::None:
      default:
        return 0; // lets ru
    }
  }

  inline static std::string stringify(Instr instr) {
    struct InstrVisitor {
      std::string operator()(MovInstr instr) const {
        if (instr.destSize != instr.srcSize) {
          // Operands are different sizes...
          // movzx dest, src
        }
        std::stringstream ss; // C++  is trash why did i quit zig
        ss << "mov" << dsToChar(instr.srcSize) << ' ' << instr.src << ", " << instr.dest << "\n\t";
        return ss.str();
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
        return "add " + instr.rhs + ", " + instr.lhs + "\n\t";
      }
      std::string operator()(SubInstr instr) const {
        return "sub " + instr.rhs + ", " + instr.lhs + "\n\t";
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
      std::string operator()(CallInstr instr) const {
        return "call " + instr.name + "\n\t";
      }
      std::string operator()(Syscall instr) const {
        return "syscall # " + instr.name + "\n\t";
      }
      std::string operator()(NegInstr instr) const {
        return "neg " + instr.what + "\n\t";
      }
      std::string operator()(NotInstr instr) const {
        return "not " + instr.what + "\n\t";
      }
      std::string operator()(DBInstr instr) const {
        return "db " + instr.what + "\n\t";
      }
      std::string operator()(AscizInstr instr) const {
        return ".asciz " + instr.what + "\n\t";
      }
      std::string operator()(Ret instr) const { return "ret\n\t"; }
      std::string operator()(Comment instr) const {
        return "# " + instr.comment + "\n\t";
      }
      std::string operator()(LinkerDirective instr) const { return instr.value; } // It is the responsibility of LinkerDirective to have its own formatting
    };
    return std::visit(InstrVisitor {}, instr.var);
  }
};