#pragma once

#include <string>
#include <unordered_map>
#include <variant>

enum class JumpCondition;

enum class DataSize {
  None,  // None/auto type
  Byte,  // 8 bits
  Word,  // 16 bits
  Dword, // 32 bits
  Qword, // 64 bits
  SS,    // 24-bits - yes its weird. Scalar single-precision float.
};

struct MovInstr {
  std::string dest;
  std::string src;
  DataSize destSize = DataSize::Qword; // "movq", "movb", etc...
  DataSize srcSize = DataSize::Qword;  // "movq $15, 0(%rsp)"
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

struct BinaryInstr {
  std::string op;
  std::string src;
  std::string dst;
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

// struct SetInstr {
//   std::string what;
//   std::string where;
// };

struct DataSectionInstr {
  DataSize bytesToDefine;
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
  std::string
      value; // This instruction is effectively pushing a string to the file.
};

struct AscizInstr {
  std::string what;
};

enum class ConvertType {
  SI2SS,  // single int to scalar single-precision
  SI2SD,  // single int to scalar double-precision
  TSS2SI, // truncate scalar single-precision to single int
  TSD2SI, // truncate scalar double-precision to single int
  SS2SD,  // scalar single-precision to scalar double-precision
  SD2SS,  // scalar double-precision to scalar single-precision
  SS2SI,  // scalar single-precision to single int
  SD2SI   // scalar double-precision to single int
};

struct ConvertInstr {
  ConvertType convType;
  std::string from;
  std::string to;
};

enum class InstrType {
  Binary,
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

  Convert,

  // Types
  Ascii,
  Asciz,
  DB,

  Byte,
  Word,
  Dword, // actually factually its ".long" hahahhaha
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
               MulInstr, DivInstr, CmpInstr, Label, Syscall, Ret, NegInstr,
               NotInstr, JumpInstr, Comment, DataSectionInstr, CallInstr,
               LinkerDirective, AscizInstr, BinaryInstr, ConvertInstr>
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
    {InstrType::Mov, InstrType::Mov},
    {InstrType::Convert, InstrType::Convert},
    {InstrType::DB, InstrType::DB},
    {InstrType::Binary, InstrType::Binary},
    {InstrType::Ascii, InstrType::Asciz},
    {InstrType::Asciz, InstrType::Ascii},
    {InstrType::Byte, InstrType::Byte},
    {InstrType::Word, InstrType::Word},
    {InstrType::Dword, InstrType::Dword},
    {InstrType::Qword, InstrType::Qword},

    // system (no opposites)
    {InstrType::Jmp, InstrType::NONE}, // if you jumped, you can't "unjump"
    {InstrType::Label, InstrType::NONE},
    {InstrType::Syscall, InstrType::NONE},
    {InstrType::Ret, InstrType::NONE},
    {InstrType::Comment, InstrType::NONE},
    {InstrType::Call, InstrType::NONE},
    {InstrType::Linker, InstrType::Linker},
    {InstrType::NONE, InstrType::NONE}
};