#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum class JumpCondition;

enum class DataSize {
  None,   // None/auto type
  Byte,   // 8 bits
  Word,   // 16 bits
  Dword,  // 32 bits
  Qword,  // 64 bits
};


struct MovInstr {
  std::string dest;
  std::string src;
  DataSize destSize = DataSize::Qword; // "movq", "movb", etc...
  DataSize srcSize = DataSize::Qword; // "movq $15, 0(%rsp)"
};

struct PushInstr {
  std::string what;
  DataSize whatSize = DataSize::Qword;
};

struct PopInstr {
  std::string where;
  DataSize whereSize = DataSize::Qword; // "popq", "popb", etc..
};

struct XorInstr {
  std::string lhs;
  std::string rhs;
};

struct AddInstr {
  std::string lhs;
  std::string rhs;
};

struct SubInstr {
  std::string lhs;
  std::string rhs;
};

struct MulInstr {
  std::string from;
};

struct DivInstr {
  std::string from;
};

struct Label {
  std::string name;
};

struct CmpInstr {
  std::string lhs;
  std::string rhs;
};

struct NegInstr {
  std::string what;
};

struct NotInstr {
  std::string what;
};

struct JumpInstr {
  JumpCondition op;
  std::string label;
};

struct SetInstr {
  std::string what;
  std::string where;
};

struct DBInstr {
  std::string what;
};

struct CallInstr {
  std::string name;
};

struct Syscall {
  std::string name;
};

struct Ret {};

struct Comment {
  std::string comment;
};

struct LinkerDirective {
  std::string value; // This instruction is effectively pushing a string to the file.
};

enum class InstrType {
  Push,
  Pop,
  Xor,
  Add,
  Sub,
  Mul,
  Div,
  Jmp,
  Cmp,
  Set,
  Neg,
  Not,

  // Types
  Ascii,
  Asciz,
  
  Byte,
  Word,
  Dword,
  Qword,

  // system (no opposites)
  Label,
  Linker,
  Call,
  Mov,
  Syscall,
  Ret,
  Comment,
  NONE
};

enum class JumpCondition {
  // just a "jump" label
  Unconditioned,

  Equal,
  NotEqual,

  Zero,
  NotZero,

  Greater,
  GreaterEqual,
  NotGreater, // Literally just LessEqual, but somehow it is different keyword
              // in ASM

  Less,
  LessEqual,
  NotLess, // just GreaterEqual, but different keyword
};

struct Instr {
  std::variant<MovInstr, PushInstr, PopInstr, XorInstr, AddInstr, SubInstr,
               MulInstr, DivInstr, CmpInstr, SetInstr, Label, Syscall, Ret, 
               NegInstr, NotInstr, JumpInstr, Comment, DBInstr, CallInstr,
              LinkerDirective>
      var;
  InstrType type;
  bool optimize = true;
};

inline std::unordered_map<InstrType, InstrType> opposites = {
    {InstrType::Push, InstrType::Pop},
    {InstrType::Pop, InstrType::Push},

    {InstrType::Xor, InstrType::Xor},

    {InstrType::Add, InstrType::Sub},
    {InstrType::Sub, InstrType::Add},

    {InstrType::Mul, InstrType::Div},
    {InstrType::Div, InstrType::Mul},

    {InstrType::Cmp, InstrType::Cmp},
    {InstrType::Set, InstrType::Set},

    {InstrType::Neg, InstrType::Neg},
    {InstrType::Not, InstrType::Not},
    // Types
    {InstrType::Ascii, InstrType::Asciz},
    {InstrType::Asciz, InstrType::Ascii},
    {InstrType::Byte, InstrType::Byte},
    {InstrType::Word, InstrType::Word},
    {InstrType::Dword, InstrType::Dword},
    {InstrType::Qword, InstrType::Qword},

    // system (no opposites)
    {InstrType::Jmp, InstrType::NONE}, // if you jumped, you can't "unjump"
    {InstrType::Label, InstrType::NONE},
    {InstrType::Mov, InstrType::NONE},
    {InstrType::Syscall, InstrType::NONE},
    {InstrType::Ret, InstrType::NONE},
    {InstrType::Comment, InstrType::NONE},
    {InstrType::Call, InstrType::NONE},
    {InstrType::Linker, InstrType::Linker},
    {InstrType::NONE, InstrType::NONE}

};
class Optimizer {
public:
  static std::vector<Instr> optimizeInstrs(std::vector<Instr> &input) {
    std::vector<Instr> output{};

    Instr prev = {.type = InstrType::NONE};

    for (Instr &instr : input) {
      if (prev.type == opposites.at(instr.type)) {
        if (instr.type == InstrType::Pop) {
          // previous was push, so we could simplify to mov
          output.pop_back();

          PushInstr prevAsPush = std::get<PushInstr>(prev.var);
          PopInstr currAsPop = std::get<PopInstr>(instr.var);

          if (prevAsPush.what == currAsPop.where)
            continue; // No 🫴

          if (prevAsPush.what == "0" &&
              (currAsPop.where.find('[') == std::string::npos)) {
            // simplify further to XOR
            Instr newInstr = {
                .var = XorInstr{.lhs = currAsPop.where, .rhs = currAsPop.where},
                .type = InstrType::Xor};
            prev = newInstr;
            output.push_back(newInstr);
            continue;
          }

          // simplify to mov
          Instr newInstr = {
              .var = MovInstr{.dest = currAsPop.where, .src = prevAsPush.what, .destSize = currAsPop.whereSize, .srcSize = prevAsPush.whatSize},
              .type = InstrType::Mov};
            
          prev = newInstr; // check expr's, "newInstr" is exactly what it should be
          output.push_back(newInstr);
          continue;
        }
      }
      output.push_back(instr);
      prev = instr;
    }

    return output;
  }
};
