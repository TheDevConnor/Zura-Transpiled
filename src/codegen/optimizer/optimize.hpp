#pragma once

#include "instr.hpp"

#include <vector>

class Optimizer {
public:
    template <typename T>
    static void tryPop(std::vector<T> *out) {
      if (out->empty()) return;
      out->pop_back();
    }

    static void optimizePair(std::vector<Instr> *output, Instr &prev, Instr &curr);
    static std::vector<Instr> optimizeInstrs(std::vector<Instr> &input);

private:
    static void appendAndResetPrev(std::vector<Instr> *output, Instr &curr, Instr &prev);
    static void processMov(std::vector<Instr> *output, Instr &prev, Instr &curr);
    static void simplifyPushPopPair(std::vector<Instr> *output, Instr &prev, Instr &curr);
    static void processOppositePair(std::vector<Instr> *output, Instr &prev, Instr &curr);
    static void optimizeSpacedPairs(std::vector<Instr> &firstPass);
    static void simplifyDebug(std::vector<Instr> *output, Instr &prev, Instr &curr);
    static bool isSameMov(const MovInstr &prev, const MovInstr &curr);
    static bool isOppositeMov(const MovInstr &prev, const MovInstr &curr);
    static bool shouldIgnorePushPop(const std::string &reg);
    static inline int previousDebugLine = 0;
};