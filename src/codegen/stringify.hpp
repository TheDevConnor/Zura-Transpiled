#pragma once

#include "optimize.hpp"

#include <string>
#include <vector>
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

  // dataSize to Char
  inline static std::string dsToChar(DataSize in) {
    /* good to have for a quick look
    NONE,  0
    BYTE,  1
    WORD,  2
    DWORD, 3
    QWORD, 4 
    */
    switch (in) {
      case DataSize::Byte:
        return "b";
      case DataSize::Word:
        return "w";
      case DataSize::Dword:
        return "l"; // 'long' - aka 'dword'
      case DataSize::Qword:
        return "q";
      case DataSize::None:
      default:
        return ""; // "I dont know", but usually the assembler can assume types
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
        return "push" + dsToChar(instr.whatSize) + " " + instr.what + "\n\t";
      }
      std::string operator()(PopInstr instr) const {
        return "pop" + dsToChar(instr.whereSize) + " " + instr.where + "\n\t";
      }
      std::string operator()(XorInstr instr) const {
        // Assume qword
        return "xorq " + instr.lhs + ", " + instr.rhs + "\n\t";
      }
      std::string operator()(AddInstr instr) const {
        return "addq " + instr.rhs + ", " + instr.lhs + "\n\t";
      }
      std::string operator()(SubInstr instr) const {
        return "subq " + instr.rhs + ", " + instr.lhs + "\n\t";
      }
      std::string operator()(MulInstr instr) const {
        return "mulq " + instr.from + "\n\t";
      }
      std::string operator()(DivInstr instr) const {
        return "divq " + instr.from + "\n\t";
      }
      std::string operator()(Label instr) const {
        return "\n" + instr.name + ":\n\t";
      }
      std::string operator()(CmpInstr instr) const {
        // For some GODFORSAKEN reason, AT&T syntax switches `cmp` instruction operands, too!
        /*
        cmpq $8, $16
        jg example # JUMPS IF 16 > 8 ???!?!?
        */
        return "cmpq " + instr.rhs + ", " + instr.lhs + "\n\t";
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
          case JumpCondition::LessEqual:
            keyword = "jle";
            break;
          case JumpCondition::Less:
            keyword = "jl";
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
      // connor was on something bro lmao
      // set is not an instruction :joy:
      // std::string operator()(SetInstr instr) const {
      //   return "set" + instr.what + " " + instr.where + "\n\t";
      // }
      std::string operator()(CallInstr instr) const {
        return "call " + instr.name + "\n\t";
      }
      std::string operator()(Syscall instr) const {
        return "syscall # " + instr.name + "\n\t";
      }
      // 2's complement - negate a reg / effective addr
      std::string operator()(NegInstr instr) const {
        return "negq " + instr.what + "\n\t";
      }
      // bitwise not
      std::string operator()(NotInstr instr) const {
        return "not " + instr.what + "\n\t";
      }
      // define bytes (intel syntax, replaced by .asciz)
      std::string operator()(DBInstr instr) const {
        return "db " + instr.what + "\n\t";
      }
      // define an ascii string with null (zero) termination
      std::string operator()(AscizInstr instr) const {
        return ".asciz " + instr.what + "\n\t";
      }
      // return from %rip and trace back up the call stack
      std::string operator()(Ret instr) const { return "ret\n\t"; }
      // self explanatory
      std::string operator()(Comment instr) const {
        return "# " + instr.comment + "\n\t";
      }
      // binary operation (Add, Sub, Mul, Div, ...)
      std::string operator()(BinaryInstr instr) const {
        std::string inst = instr.op + " " + instr.src;
        if (instr.dst != "") {
          inst += ", " + instr.dst;
        }
        return inst + "\n\t";
      }
      // String literal (eg .cfi_startproc in functions)
      // It is the responsibility of the input to have its own formatting (\n\t)
      std::string operator()(LinkerDirective instr) const { return instr.value; }
    };
    return std::visit(InstrVisitor {}, instr.var);
  }
};