#pragma once

#include "instr.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class Stringifier { // converts Instr structures into AT&T Syntax strings
public:
  inline static std::string stringifyInstrs(std::vector<Instr> &input, bool debug = false) {
    std::string output{};
    for (Instr &instr : input) {
      // If in normal mode and the intsruction is a comment, skip it
      // Otherwise, print it
      // This is because we want as much information as possible in debug mode,
      // but we do not care about comments when printing regular, optimized assembly.
      if (!debug && instr.type == InstrType::Comment) continue;
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
    SS,    5
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
      case DataSize::SS:
        return "ss";
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
      std::string operator()(LeaInstr instr) const {
        return "lea" + dsToChar(instr.size) + " " + instr.src + ", " + instr.dest + "\n\t";
      };
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
      // define bytes 
      std::string operator()(DataSectionInstr instr) const {
        std::string op = "";
        switch (instr.bytesToDefine) {
          case DataSize::Byte:
            op = ".byte";
            break;
          case DataSize::Word:
            op = ".word";
            break;
          case DataSize::Dword:
          case DataSize::SS:
            op = ".long";
            break;
          case DataSize::None: // assume qword (i mean, this is x86-64 architecture after all)
          case DataSize::Qword:
          // case DataSize::DS:
            op = ".qword";
            break;
          default:
            op = "UNIMPLEMENTED";
            break;
        }
        return op + " " + instr.what + "\n\t";
      }
      // define an ascii (UTF-8) string with null (zero) termination
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

      std::string operator()(ConvertInstr instr) const {
        std::string inst = "cvt";
        switch (instr.convType) {
          case ConvertType::SI2SS: // single int to scalar single-precision
            inst += "si2ss";
            break;
          case ConvertType::SS2SI: // scalar single-precision to single int
            inst += "ss2si";
            break;
          case ConvertType::SD2SI: // scalar double-precision to single int
            inst += "sd2si";
            break;
          case ConvertType::SI2SD: // single int to scalar double-precision
            inst += "si2sd";
            break;
          case ConvertType::SS2SD: // scalar single-precision to scalar double-precision
            inst += "ss2sd";
            break;
          case ConvertType::SD2SS: // scalar double-precision to scalar single-precision
            inst += "sd2ss";
            break;
          case ConvertType::TSD2SI: // truncate scalar double-precision to single int
            inst += "tsd2si";
            break;
          case ConvertType::TSS2SI: // truncate scalar single-precision to single int
            inst += "tss2si";
            break;
          default:
            std::cerr << "Unimplemnted ConvertType [" << (int)instr.convType << "]" << std::endl;
            return "# unimplented cvt. :(\n\t";
        }
        return inst + " " + instr.from + ", " + instr.to + "\n\t";
      }
      // String literal (eg .cfi_startproc in functions)
      // It is the responsibility of the input to have its own formatting (\n\t)
      std::string operator()(LinkerDirective instr) const { return instr.value; }
    };
    return std::visit(InstrVisitor {}, instr.var);
  }
};