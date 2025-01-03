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

// Dest, src, dSize, sSize
struct MovInstr {
  std::string dest;
  std::string src;
  DataSize destSize = DataSize::Qword; // "movq", "movb", etc...
  DataSize srcSize = DataSize::Qword;  // "movq $15, 0(%rsp)"
};

// Size, dest, src
struct LeaInstr {
  DataSize size; // "leaq", "leaw", etc...
  std::string dest;
  std::string src;
};

// what, wSize
struct PushInstr {
  std::string what;
  DataSize whatSize = DataSize::Qword;
};

// where, wSize
struct PopInstr {
  std::string where;
  DataSize whereSize = DataSize::Qword; // "popq", "popb", etc..
};

// lhs, rhs
struct XorInstr {
  std::string lhs;
  std::string rhs;
};

// op, src, dst
struct BinaryInstr {
  std::string op;
  std::string src;
  std::string dst;
};

// lhs, rhs
struct AddInstr {
  std::string lhs;
  std::string rhs;
};

// lhs, rhs
struct SubInstr {
  std::string lhs;
  std::string rhs;
};

// from
struct MulInstr {
  std::string from;
};

// from
struct DivInstr {
  std::string from;
};

// name
struct Label {
  std::string name;
};

// lhs, rhs
struct CmpInstr {
  std::string lhs;
  std::string rhs;
};

// what
struct NegInstr {
  std::string what;
};

// what
struct NotInstr {
  std::string what;
};

// op, label
struct JumpInstr {
  JumpCondition op;
  std::string label;
};

// what, where
// struct SetInstr {
//   std::string what;
//   std::string where;
// };

// bytesToDefine, what
struct DataSectionInstr {
  DataSize bytesToDefine;
  std::string what;
};

// name
struct CallInstr {
  std::string name;
};

// name (SYS_EXIT, SYS_WRITE, etc.)
struct Syscall {
  std::string name;
};

// fromWhere
struct Ret {
  std::string fromWhere;
};

// comment
struct Comment {
  std::string comment;
};

// value
struct LinkerDirective {
  std::string
      value; // This instruction is effectively pushing a string to the file.
};

// what
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
  // NOTE: packed registers are not supported because they are too complicated to work with
};

// convType, from, to
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
  Lea,
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
               LinkerDirective, AscizInstr, BinaryInstr, ConvertInstr, LeaInstr>
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
    {InstrType::Lea, InstrType::Lea},
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