#include "optimize.hpp"

void Optimizer::optimizePair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    if (!prev.optimize || !curr.optimize) {
        appendAndResetPrev(output, curr, prev);
        return;
    }

    if (curr.type == InstrType::Comment)
        return;

    if (curr.type == InstrType::Mov) {
        processMov(output, prev, curr);
    } else if (prev.type == opposites.at(curr.type)) {
        processOppositePair(output, prev, curr);
    } else {
        output->push_back(curr);
    }
}

std::vector<Instr> Optimizer::optimizeInstrs(std::vector<Instr> &input) {
    if (input.empty())
        return {};

    std::vector<Instr> firstPass;
    Instr prev = {.type = InstrType::NONE};

    for (Instr &instr : input) {
        optimizePair(&firstPass, prev, instr);
        prev = instr;
    }

    removePushPopPairs(firstPass);
    return firstPass;
}

void Optimizer::appendAndResetPrev(std::vector<Instr> *output, Instr &curr, Instr &prev) {
    output->push_back(curr);
    prev = Instr {.type=InstrType::NONE};
}

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

bool Optimizer::isSameMov(const MovInstr &prev, const MovInstr &curr) {
    return prev.dest == curr.dest && prev.src == curr.src;
}

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

void Optimizer::simplifyPushPopPair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    PushInstr prevAsPush = std::get<PushInstr>(prev.var);
    PopInstr currAsPop = std::get<PopInstr>(curr.var);

    if (prevAsPush.what == currAsPop.where) {
        prev = Instr {.type=InstrType::NONE};
        return;
    }

    if (prevAsPush.what == "0" && currAsPop.where.find('[') == std::string::npos) {
        Instr newInstr = {.var = XorInstr{.lhs = currAsPop.where, .rhs = currAsPop.where}, .type = InstrType::Xor};
        prev = newInstr;
        output->push_back(newInstr);
    } else {
        Instr newInstr = {
            .var = MovInstr{.dest = currAsPop.where, .src = prevAsPush.what, .destSize = currAsPop.whereSize, .srcSize = prevAsPush.whatSize},
            .type = InstrType::Mov
        };
        prev = newInstr;
        output->push_back(newInstr);
    }
}

void Optimizer::removePushPopPairs(std::vector<Instr> &firstPass) {
    Instr prev = {.type = InstrType::NONE};
    int prevIndex = 0;

    for (int i = 0; i < firstPass.size(); ++i) {
        Instr &instr = firstPass[i];

        if (instr.type == InstrType::Push) {
            PushInstr push = std::get<PushInstr>(instr.var);
            if (shouldIgnorePushPop(push.what)) continue;
            prev = instr;
            prevIndex = i;
        } else if (instr.type == InstrType::Pop) {
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

bool Optimizer::shouldIgnorePushPop(const std::string &reg) {
    return reg == "%rbp" || reg == "%rsp";
}