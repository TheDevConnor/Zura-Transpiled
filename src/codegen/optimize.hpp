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
  std::string value; // This instruction is effectively pushing a string to the file.
};

struct AscizInstr {
  std::string what;
};

enum class ConvertType {
  SI2SS, // single int to scalar single-precision
  SI2SD, // single int to scalar double-precision
  TSS2SI, // truncate scalar single-precision to single int
  TSD2SI, // truncate scalar double-precision to single int
  SS2SD, // scalar single-precision to scalar double-precision
  SD2SS, // scalar double-precision to scalar single-precision
  SS2SI, // scalar single-precision to single int
  SD2SI // scalar double-precision to single int
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
               MulInstr, DivInstr, CmpInstr, Label, Syscall, Ret, 
               NegInstr, NotInstr, JumpInstr, Comment, DataSectionInstr, CallInstr,
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

/*

This is a static class that transforms UGLY push/pop pairs into neat little mov's.
I mean, look at an unoptimized code example.

- pushq %rax
- pushq %rbx
- popq %rcx
- popq %rdx
Now optimized:
- movq %rdx, %rax
- movq %rcx, %rbx

*/
class Optimizer {
public:
  
  template <typename T>
  static void tryPop(std::vector<T> *out) {
    if (out->size() == 0) return;
    out->pop_back();
  }

  static void optimizePair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
      if (!prev.optimize || !curr.optimize) { // well, it doesnt want it
        output->push_back(curr);
        prev = Instr {.type=InstrType::NONE};
        return;
      }
      if (curr.type == InstrType::Comment) return; // TEMPORARY! Trying to see if this works
      if (curr.type == InstrType::Mov) {
        MovInstr currAsMov = std::get<MovInstr>(curr.var);
        if (currAsMov.dest == currAsMov.src) return; // why??
        if (prev.type == InstrType::Mov) {
          // prev was mov, too!
          MovInstr prevAsMov = std::get<MovInstr>(prev.var);
          if (prevAsMov.dest == currAsMov.dest
            && prevAsMov.src == currAsMov.src) {
            // remove previous
            tryPop<Instr>(output);
            prev = Instr {.type=InstrType::NONE};
            return;
          }
          
          // check for "opposite" pairs
          // ie
          // movq %rax, %rdi
          // movq %rdi, %rax
          if (prevAsMov.dest == currAsMov.src && prevAsMov.src == currAsMov.dest) {
            // remove both
            tryPop<Instr>(output);
            prev = Instr {.type=InstrType::NONE};
            return;
          }
        }
        // Clearly, there was no reason not to push it
        // So let's do that!
        output->push_back(curr);
      }
      else if (prev.type == opposites.at(curr.type)) {
        if (curr.type == InstrType::Pop) { // check for push/pop pairs
          // previous was push, so we could simplify to mov
          tryPop<Instr>(output);

          PushInstr prevAsPush = std::get<PushInstr>(prev.var);
          PopInstr currAsPop = std::get<PopInstr>(curr.var);

          if (prevAsPush.what == currAsPop.where) { // push rax, pop rax is really stupid so ignore them
            prev = Instr {.type=InstrType::NONE};
            return; // No 🫴
          }

          if (prevAsPush.what == "0" &&
              (currAsPop.where.find('[') == std::string::npos)) {
            // simplify further to XOR
            Instr newInstr = {
                .var = XorInstr{.lhs = currAsPop.where, .rhs = currAsPop.where},
                .type = InstrType::Xor};
            prev = newInstr;
            output->push_back(newInstr);
            return;
          }

          // simplify to mov
          Instr newInstr = {
              .var = MovInstr{.dest = currAsPop.where, .src = prevAsPush.what, .destSize = currAsPop.whereSize, .srcSize = prevAsPush.whatSize},
              .type = InstrType::Mov};
            
          prev = newInstr; // check expr's, "newInstr" is exactly what it should be
          output->push_back(newInstr);
          return;
        } else output->push_back(curr);
      }
      else output->push_back(curr);
  }
  static std::vector<Instr> optimizeInstrs(std::vector<Instr> &input) {
    if (input.size() == 0) return {};
    std::vector<Instr> firstPass = {};

    Instr prev = {.type = InstrType::NONE};

    for (Instr &instr : input) {
      optimizePair(&firstPass, prev, instr);
      prev = instr;
    }

    /*
    make sure:
    - pushq %rax
    - movq %rbx, $8
    - popq %rcx

    becomes:
    - movq %rbx, $8
    - movq %rcx, %rax

    NOTE: This optimization is inefficient. It is mainly for binary operations...
    binary-operation 4 + 2:
    - pushq $4
    - movq %rax, $2
    - popq %rbx
    It's hard on the eyes, right?
    */
    prev = Instr {.type=InstrType::NONE};
    int prevIndex = 0;
    for (int i = 0; i < firstPass.size(); i++) {
      Instr instr = firstPass[i];
      if (instr.type == InstrType::Push) {
        PushInstr push = std::get<PushInstr>(instr.var);
        if (push.what == "%rbp" || push.what == "%rsp") continue; // Push/pop with rbp is NOT tweaked by codegen outside of function prologue/epilogue. DO NOT OPMTIMIZE THAT!!
        prev = instr;
        prevIndex = i;
      } else if (instr.type == InstrType::Pop) {
        PopInstr pop = std::get<PopInstr>(instr.var);
        PushInstr push = std::get<PushInstr>(prev.var);
        if (pop.where == "%rbp" || pop.where == "%rsp") continue; // Earlier comment lmao
        firstPass.erase(firstPass.cbegin() + prevIndex);
        firstPass.erase(firstPass.cbegin() + i - 1);
        Instr newInstr = {
          .var = MovInstr{.dest = pop.where, .src = push.what, .destSize = pop.whereSize, .srcSize = push.whatSize},
          .type = InstrType::Mov
        };
        firstPass.insert(firstPass.begin() + prevIndex, newInstr);
        i = prevIndex;
        prev = Instr {.type=InstrType::NONE};
      }
    }

    return firstPass;
  }
};