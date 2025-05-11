#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../gen.hpp"
#include "instr.hpp"

class Stringifier {  // converts Instr structures into AT&T Syntax strings
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
    SS,    5
    */
    switch (in) {
      case DataSize::Byte:
        return "b";
      case DataSize::Word:
        return "w";
      case DataSize::Dword:
        return "l";  // 'long' - aka 'dword'
      case DataSize::Qword:
        return "q";
      case DataSize::SS:
        return "ss";
      case DataSize::None:
      default:
        return "";  // "I dont know", but usually the assembler can assume types
    }
  }

  inline static std::string stringify(Instr instr) {
    struct InstrVisitor {
      std::string operator()(MovInstr instr) const {
        if (instr.destSize != instr.srcSize) {
          // Operands are different sizes...
          // However, if one of them is a None and the other isn't, we can assume the one that is not none
          if (instr.srcSize == DataSize::None) {
            // Assume the dest is the one that is not none
            std::stringstream ss;
            ss << "mov" << dsToChar(instr.destSize) << " " << instr.src << ", " << instr.dest << "\n\t";
            return ss.str();
          } else if (instr.destSize == DataSize::None) {
            // Assume the src is the one that is not none
            std::stringstream ss;
            ss << "mov" << dsToChar(instr.srcSize) << " " << instr.src << ", " << instr.dest << "\n\t";
            return ss.str();
          }
          // movzx dest, src
          if (instr.destSize == DataSize::SD &&
              instr.srcSize == DataSize::Qword) {
            // Just go ahead with it
            std::stringstream ss;
            ss << "mov" << dsToChar(instr.destSize) << " " << instr.src << ", " << instr.dest << "\n\t";
            return ss.str();
          }
          if (instr.destSize == DataSize::SS &&
              instr.srcSize == DataSize::Dword) {
            // Just go ahead with it
            std::stringstream ss;
            ss << "mov" << dsToChar(instr.destSize) << " " << instr.src << ", " << instr.dest << "\n\t";
            return ss.str();
          }
          // Everything else, we must assume a zero-extend
          std::stringstream ss;
          ss << "movzx" << dsToChar(instr.srcSize) << " " << instr.src << ", " << instr.dest << "\n\t";
          return ss.str();
        }
        if (instr.src.starts_with("$") && instr.src.size() > 1) {
          if (std::isdigit(instr.src[1])) {
            // it's a number! if its larger than 2^32, we must use movabsq
            if (std::stoll(instr.src.substr(1)) > 4294967295) {
              std::stringstream ss;
              // in this case, i highly doubt that the destSize will be anything other than Qword
              ss << "movabs" << dsToChar(instr.destSize) << ' ' << instr.src << ", " << instr.dest << "\n\t";
              return ss.str();
            }
          }
        }
        std::stringstream ss;
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
        return "add" + dsToChar(instr.size) + " " + instr.rhs + ", " + instr.lhs + "\n\t";
      }
      std::string operator()(LeaInstr instr) const {
        return "lea" + dsToChar(instr.size) + " " + instr.src + ", " + instr.dest + "\n\t";
      };
      std::string operator()(SubInstr instr) const {
        return "sub" + dsToChar(instr.size) + " " + instr.rhs + ", " + instr.lhs + "\n\t";
      }
      std::string operator()(MulInstr instr) const {
        if (instr.isSigned) {
          return "imul" + dsToChar(instr.size) + " " + instr.from + "\n\t";
        }
        return "mul" + dsToChar(instr.size) + " " + instr.from + "\n\t";
      }
      std::string operator()(DivInstr instr) const {
        if (instr.isSigned) {
          return "idiv" + dsToChar(instr.size) + " " + instr.from + "\n\t";
        }
        return "div" + dsToChar(instr.size) + " " + instr.from + "\n\t";
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
        return "cmp" + dsToChar(instr.size) + " " + instr.rhs + ", " + instr.lhs + "\n\t";
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
        return "neg" + dsToChar(instr.size) + " " + instr.what + "\n\t";
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
            op = ".long";
            break;
          case DataSize::None:  // assume qword (i mean, this is x86-64 architecture after all)
          case DataSize::Qword:
            // case DataSize::DS:
            op = ".qword";
            break;
          case DataSize::SD:
            op = ".double";
            break;
          case DataSize::SS:
            op = ".float";
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
      std::string operator()(Ret instr) const { return "ret # " + instr.fromWhere + "\n\t"; }
      // self explanatory
      std::string operator()(Comment instr) const {
        if (codegen::debug)
          return "# " + instr.comment + "\n\t";
        return "";
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
          case ConvertType::SI2SS:  // single int to scalar single-precision
            inst += "si2ss";
            break;
          case ConvertType::SS2SI:  // scalar single-precision to single int
            inst += "ss2si" + dsToChar(instr.toSize);
            break;
          case ConvertType::SD2SI:  // scalar double-precision to single int
            inst += "sd2si" + dsToChar(instr.toSize);
            break;
          case ConvertType::SI2SD:  // single int to scalar double-precision
            inst += "si2sd";
            break;
          case ConvertType::SS2SD:  // scalar single-precision to scalar double-precision
            inst += "ss2sd";
            break;
          case ConvertType::SD2SS:  // scalar double-precision to scalar single-precision
            inst += "sd2ss";
            break;
          case ConvertType::TSD2SI:  // truncate scalar double-precision to single int
            inst += "tsd2si" + dsToChar(instr.toSize);
            break;
          case ConvertType::TSS2SI:  // truncate scalar single-precision to single int
            inst += "tss2si" + dsToChar(instr.toSize);
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
    return std::visit(InstrVisitor{}, instr.var);
  }
};
