#include "optimize.hpp"

void Optimizer::optimizePair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    if (!prev.optimize || !curr.optimize) {
        appendAndResetPrev(output, curr, prev);
        return;
    }

    // A temporary change - might add future compiler options to re-enable this later
    if (curr.type == InstrType::Comment)
        return;

    if (curr.type == InstrType::Mov) {
        processMov(output, prev, curr);
    } else if (curr.type == InstrType::Linker) {
        simplifyDebug(output, curr);
    } else if (prev.type == opposites.at(curr.type)) {
        processOppositePair(output, prev, curr);
    } else {
        output->push_back(curr);
    }
}

// Will run more instruction-specific optimizations depending on the type of the pair.
std::vector<Instr> Optimizer::optimizeInstrs(std::vector<Instr> &input) {
    if (input.empty())
        return {};

    std::vector<Instr> firstPass;
    Instr prev = {.type = InstrType::NONE};

    // Optimizes touching pairs
    /* asm
    pushq $4
    popq %rax
    */
    for (Instr &instr : input) {
        optimizePair(&firstPass, prev, instr);
        prev = instr;
    }

    // Ex:
    /* asm
    pushq $4
    # Comment getting the way
    popq %rax
    */
   // will get optimized here
    optimizeSpacedPairs(firstPass);
    return firstPass;
}

// As the name says.
void Optimizer::appendAndResetPrev(std::vector<Instr> *output, Instr &curr, Instr &prev) {
    output->push_back(curr);
    prev = Instr {.type=InstrType::NONE};
}

// Checks if mov instructions are redundant
void Optimizer::processMov(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    MovInstr currAsMov = std::get<MovInstr>(curr.var);

    if (currAsMov.dest == currAsMov.src) return;

    if (prev.type == InstrType::Mov) {
        MovInstr prevAsMov = std::get<MovInstr>(prev.var);

        if (isSameMov(prevAsMov, currAsMov) || isOppositeMov(prevAsMov, currAsMov)) {
            tryPop(output);
            prev = Instr {.type=InstrType::NONE};
            return;
        }
    }

    output->push_back(curr);
}

// Are the 2 mov instructions exactly the same src and dst
bool Optimizer::isSameMov(const MovInstr &prev, const MovInstr &curr) {
    return prev.dest == curr.dest && prev.src == curr.src;
}

// Are the 2 mov instructions opposites, meaning nothing has changed
bool Optimizer::isOppositeMov(const MovInstr &prev, const MovInstr &curr) {
    return prev.dest == curr.src && prev.src == curr.dest;
}

void Optimizer::processOppositePair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    if (curr.type == InstrType::Pop) {
        tryPop(output);
        simplifyPushPopPair(output, prev, curr);
    } else {
        output->push_back(curr);
    }
}

void Optimizer::simplifyDebug(std::vector<Instr> *output, Instr &curr) {
    LinkerDirective currAsLinker = std::get<LinkerDirective>(curr.var);
    if (currAsLinker.value.find(".loc") == std::string::npos) return output->push_back(curr);
    int currDebugLine = std::stoi(currAsLinker.value.substr(7));
    if (currDebugLine == previousDebugLine) return;
    previousDebugLine = currDebugLine;
    output->push_back(curr);
}

// Turn push/pops into mov's or xor's
void Optimizer::simplifyPushPopPair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    PushInstr prevAsPush = std::get<PushInstr>(prev.var);
    PopInstr currAsPop = std::get<PopInstr>(curr.var);

    if (prevAsPush.what == currAsPop.where && prevAsPush.whatSize == currAsPop.whereSize) {
        prev = Instr {.type=InstrType::NONE};
        return;
    }

    // NOTE: xor instruction cannot handle effective addresses (this is why we checked for the paren)
    if (prevAsPush.what == "$0" && currAsPop.where.find('(') == std::string::npos) {
        Instr newInstr = {.var = XorInstr{.lhs = currAsPop.where, .rhs = currAsPop.where}, .type = InstrType::Xor};
        prev = newInstr;
        output->push_back(newInstr);
        return;
    }

    if (currAsPop.whereSize != DataSize::SS && prevAsPush.whatSize == DataSize::SS) {
        // If this is an effective address, use a regular mov
        if (currAsPop.where.find('(') == std::string::npos) {
            // The compiler would know when to convert and when not to.
            // We have to copy the bits of the float **directly** into the register.

            // movss %xmm0, (%where)
            // movq (%where), %where

            Instr firstManip = {
                .var = MovInstr{.dest = "(" + currAsPop.where + ")", .src = prevAsPush.what, .destSize = currAsPop.whereSize, .srcSize = prevAsPush.whatSize},
                .type = InstrType::Mov,
                .optimize = false
            };
            output->push_back(firstManip);

            Instr secondManip = {
                .var = MovInstr{.dest = currAsPop.where, .src = "(" + currAsPop.where + ")", .destSize = currAsPop.whereSize, .srcSize = currAsPop.whereSize},
                .type = InstrType::Mov,
                .optimize = false
            };
            output->push_back(secondManip);

            prev = {}; // This manipulation is necessary!
            return;
        }
    }

    // check if one is float register and one is int register
    if (currAsPop.whereSize == DataSize::SS && prevAsPush.whatSize != DataSize::SS) {
        Instr cvtInstr = {
            .var = ConvertInstr{ .convType = ConvertType::SS2SI, .from = currAsPop.where, .to = currAsPop.where },
            .type = InstrType::Convert
        };
        output->push_back(cvtInstr);
        prev = cvtInstr;
        return;
    }
    Instr newInstr = {
        .var = MovInstr{.dest = currAsPop.where, .src = prevAsPush.what, .destSize = currAsPop.whereSize, .srcSize = prevAsPush.whatSize},
        .type = InstrType::Mov
    };
    prev = newInstr;
    output->push_back(newInstr);
    return;
}

// Optimize pairs with a useless instruction in between
void Optimizer::optimizeSpacedPairs(std::vector<Instr> &firstPass) {
    Instr prev = {.type = InstrType::NONE};
    int prevIndex = 0;

    for (int i = 0; i < firstPass.size(); ++i) {
        Instr &instr = firstPass[i];

        if (instr.type == InstrType::Push) {
            PushInstr push = std::get<PushInstr>(instr.var);
            if (shouldIgnorePushPop(push.what)) continue;
            prev = instr;
            prevIndex = i;
        } else if (prev.type == InstrType::Push && instr.type == InstrType::Pop) {
            PopInstr pop = std::get<PopInstr>(instr.var);
            PushInstr push = std::get<PushInstr>(prev.var);
            if (shouldIgnorePushPop(pop.where)) continue;

            firstPass.erase(firstPass.begin() + prevIndex);
            firstPass.erase(firstPass.begin() + i - 1);
            
            Instr newInstr = {
                .var = MovInstr{.dest = pop.where, .src = push.what, .destSize = pop.whereSize, .srcSize = push.whatSize},
                .type = InstrType::Mov
            };
            firstPass.insert(firstPass.begin() + prevIndex, newInstr);

            i = prevIndex;
            prev = Instr {.type = InstrType::NONE};
        }
    }
}

// Normal code compiled from user's zura will never affect the stack registers
// They will be affected when it is ABSOLUTELY necessary (for exanple, function scopes)
bool Optimizer::shouldIgnorePushPop(const std::string &reg) {
    return reg == "%rbp" || reg == "%rsp";
}