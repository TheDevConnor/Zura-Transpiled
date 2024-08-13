#pragma once

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

struct MovInstr {
  std::string dest;
  std::string src;
};

struct PushInstr {
  std::string what;
};

struct PopInstr {
  std::string where;
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

struct Syscall {
  std::string name;
};

struct Ret {};

struct Comment {
  std::string comment;
};

enum class InstrType {
  Push,
  Pop,
  Xor,
  Add,
  Sub,
  Mul,
  Div,
  
  // system (no opposites)
  Label,
  Mov,
  Syscall,
  Ret,
  Comment,
  NONE
};

inline std::unordered_map<InstrType, InstrType> opposites = {
    { InstrType::Push, InstrType::Pop },
    { InstrType::Pop, InstrType::Push },

    {InstrType::Xor, InstrType::Xor},
    
    {InstrType::Add, InstrType::Sub},
    {InstrType::Sub, InstrType::Add},

    {InstrType::Mul, InstrType::Div},
    {InstrType::Div, InstrType::Mul},
    
    // system (no opposites)
    {InstrType::Label, InstrType::NONE},
    {InstrType::Mov, InstrType::NONE},
    {InstrType::Syscall, InstrType::NONE},
    {InstrType::Ret, InstrType::NONE},
    {InstrType::Comment, InstrType::NONE},
    {InstrType::NONE, InstrType::NONE}

};
class Optimezer {
public:
  struct Instr {
    std::variant<MovInstr, PushInstr, PopInstr, XorInstr, AddInstr, SubInstr,
                 MulInstr, DivInstr, Label, Syscall, Ret, Comment>
        var;
    InstrType type;
  };


  static std::vector<Instr> optimizeInstrs(std::vector<Instr> &input) {
    std::vector<Instr> output{};
    
    Instr prev = { .type = InstrType::NONE };

    for (Instr &instr : input) {
      if (prev.type == opposites.at(instr.type)) {
        if (instr.type == InstrType::Pop) {
            // previous was push, so we could simplify to mov
            output.pop_back();

            PushInstr prevAsPush = std::get<PushInstr>(prev.var);
            PopInstr currAsPop = std::get<PopInstr>(instr.var);

            if (prevAsPush.what == currAsPop.where)
                continue; // No 🫴
            
            if (prevAsPush.what == "0") {
                // simplify further to XOR
                Instr newInstr = { .var = XorInstr { .lhs = currAsPop.where, .rhs = currAsPop.where }, .type = InstrType::Xor };
                prev = newInstr;
                output.push_back(newInstr);
                continue;
            }

            // simplify to mov
            Instr newInstr = { .var = MovInstr { .dest = currAsPop.where, .src = prevAsPush.what }, .type = InstrType::Mov };
            prev = newInstr;
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
