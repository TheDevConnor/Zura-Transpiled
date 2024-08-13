#pragma once

#include "optimize.hpp"
#include <string>
#include <unordered_map>
#include <variant>
#include <stdexcept>

class Stringifier {
public:
  static std::string stringifyInstrs(const std::vector<Instr> &input) {
    std::string output{};
    for (const Instr &instr : input) {
      output += stringify(instr);
    }
    return output;
  }

  static std::string stringify(const Instr &instr) {
    // Map each InstrType to a corresponding stringification function
    static const std::unordered_map<InstrType, std::string(*)(const Instr&)> instrMap = {
        {InstrType::Mov, &Stringifier::stringifyMov},
        {InstrType::Push, &Stringifier::stringifyPush},
        {InstrType::Pop, &Stringifier::stringifyPop},
        {InstrType::Xor, &Stringifier::stringifyXor},
        {InstrType::Add, &Stringifier::stringifyAdd},
        {InstrType::Sub, &Stringifier::stringifySub},
        {InstrType::Mul, &Stringifier::stringifyMul},
        {InstrType::Div, &Stringifier::stringifyDiv},
        {InstrType::Label, &Stringifier::stringifyLabel},
        {InstrType::Syscall, &Stringifier::stringifySyscall},
        {InstrType::Ret, &Stringifier::stringifyRet},
        {InstrType::Comment, &Stringifier::stringifyComment}
    };

    auto it = instrMap.find(instr.type);
    if (it != instrMap.end()) {
      return it->second(instr) + "\n\t";
    } else {
      return "; Unknown instruction\n\t";
    }
  }

private:
  static std::string stringifyMov(const Instr &instr) {
    const auto &mov = std::get<MovInstr>(instr.var);
    return "mov " + mov.dest + ", " + mov.src;
  }

  static std::string stringifyPush(const Instr &instr) {
    const auto &push = std::get<PushInstr>(instr.var);
    return "push " + push.what;
  }

  static std::string stringifyPop(const Instr &instr) {
    const auto &pop = std::get<PopInstr>(instr.var);
    return "pop " + pop.where;
  }

  static std::string stringifyXor(const Instr &instr) {
    const auto &xorInstr = std::get<XorInstr>(instr.var);
    return "xor " + xorInstr.lhs + ", " + xorInstr.rhs;
  }

  static std::string stringifyAdd(const Instr &instr) {
    const auto &add = std::get<AddInstr>(instr.var);
    return "add " + add.lhs + ", " + add.rhs;
  }

  static std::string stringifySub(const Instr &instr) {
    const auto &sub = std::get<SubInstr>(instr.var);
    return "sub " + sub.lhs + ", " + sub.rhs;
  }

  static std::string stringifyMul(const Instr &instr) {
    const auto &mul = std::get<MulInstr>(instr.var);
    return "mul " + mul.from;
  }

  static std::string stringifyDiv(const Instr &instr) {
    const auto &div = std::get<DivInstr>(instr.var);
    return "div " + div.from;
  }

  static std::string stringifyLabel(const Instr &instr) {
    const auto &label = std::get<Label>(instr.var);
    return label.name + ":";
  }

  static std::string stringifySyscall(const Instr &instr) {
    const auto &syscall = std::get<Syscall>(instr.var);
    return "syscall ; " + syscall.name;
  }

  static std::string stringifyRet(const Instr &) {
    return "ret";
  }

  static std::string stringifyComment(const Instr &instr) {
    const auto &comment = std::get<Comment>(instr.var);
    return "; " + comment.comment;
  }
};
