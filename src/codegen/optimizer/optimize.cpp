#include "optimize.hpp"
#include "../gen.hpp"
void Optimizer::optimizePair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
  if (!prev.optimize || !curr.optimize) {
    appendAndResetPrev(output, curr, prev);
    return;
  }

  if (curr.type == InstrType::Mov) {
      processMov(output, prev, curr);
  } else if (curr.type == InstrType::Linker && prev.type == InstrType::Linker) {
      // simplifyDebug(output, prev, curr);
      output->push_back(curr);
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
    Instr prev = {.var = {}, .type = InstrType::NONE};

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
    prev = Instr {.var = {}, .type=InstrType::NONE};
}

// Checks if mov instructions are redundant
void Optimizer::processMov(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    MovInstr currAsMov = std::get<MovInstr>(curr.var);

    if (currAsMov.dest == currAsMov.src) return tryPop(output);

    if (prev.type == InstrType::Mov) {
        MovInstr prevAsMov = std::get<MovInstr>(prev.var);

        if (isSameMov(prevAsMov, currAsMov) || isOppositeMov(prevAsMov, currAsMov)) {
            tryPop(output);
            prev = Instr {.var = {}, .type=InstrType::NONE};
            return;
        }
    } else if (prev.type == InstrType::Lea) {
      processLeaMovPair(output, prev, curr);
      return;
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
        prev = Instr {.var = {}, .type=InstrType::NONE}; // Reset prev after processing
    } else if (curr.type == InstrType::Lea && prev.type == InstrType::Lea) {
        // Check if the previous lea had the same source and destination
        LeaInstr currAsLea = std::get<LeaInstr>(curr.var);
        LeaInstr prevAsLea = std::get<LeaInstr>(prev.var);
      
        if (currAsLea.dest == prevAsLea.dest &&
            !(currAsLea.src.find(prevAsLea.dest) != std::string::npos)) {
            // leaq -8(%rbp), %rax
            // leaq -8(%rbp), %rax
            // will be replaced, BUT

            // leaq -8(%rax), %rax
            // leaq -8(%rax), %rax
            // IS VERY NEEDED!!!!!!
            // Remove prev
            prev = Instr {.var = {}, .type=InstrType::NONE};
            // Try pop
            tryPop(output);
            output->push_back(curr);
        }
    } else {
        output->push_back(curr);
    }
}

// Turn push/pops into mov's or xor's
void Optimizer::simplifyPushPopPair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    PushInstr prevAsPush = std::get<PushInstr>(prev.var);
    PopInstr currAsPop = std::get<PopInstr>(curr.var);

    if (prevAsPush.what == currAsPop.where && prevAsPush.whatSize == currAsPop.whereSize) {
        prev = Instr {.var = {}, .type=InstrType::NONE};
        return;
    }

    // Check if both have same size and both have effective address ('(') 
    if (prevAsPush.whatSize == currAsPop.whereSize && prevAsPush.what.find('(') != std::string::npos && currAsPop.where.find('(') != std::string::npos) {
        // movq (%rax), %rdx
        // movq %rdx, (%rbx)
        // I hate that this is the solution, Intel go fix your stinky x86
        Instr newInstr = {
            .var = MovInstr{.dest="%r13",.src=prevAsPush.what,.destSize=DataSize::Qword,.srcSize=prevAsPush.whatSize},
            .type = InstrType::Mov
        };
        prev = newInstr;
        output->push_back(newInstr);

        Instr anotherNewInstr = {
            .var = MovInstr{.dest = currAsPop.where, .src = "%r13", .destSize = currAsPop.whereSize, .srcSize = DataSize::Qword},
            .type = InstrType::Mov
        };
        prev = newInstr;
        output->push_back(anotherNewInstr);
        prev = {}; // twitch makes me twitch bro its so annoying sometimes
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


void Optimizer::processLeaMovPair(std::vector<Instr> *output, Instr &prev, Instr &curr) {
    MovInstr currAsMov = std::get<MovInstr>(curr.var);
    LeaInstr prevAsLea = std::get<LeaInstr>(prev.var);

    /*
     * leaq -8(%rbp), %rax
     * movq %rax, %rdi
     is the same as
     * leaq -8(%rbp), %rdi
     */
    if (currAsMov.src == prevAsLea.dest) {
        // check if the dest was an effective address
        if (currAsMov.dest.find('(') != std::string::npos) {
          output->push_back(curr);
          return;
        }
        // Get rid of the mov and the leaq
        tryPop(output);
        // Create a new lea instruction with the mov's src
        Instr newInstr = {
          .var = LeaInstr{.size = prevAsLea.size, .dest = currAsMov.dest, .src = prevAsLea.src},
          .type = InstrType::Lea
        };
        output->push_back(newInstr);
        prev = Instr {.var = {}, .type=InstrType::NONE};
        return;
    } else {
        output->push_back(curr);
    }
}

// Optimize pairs with a useless instruction in between
// Ok look, I fed the previous code into chatgpt until it gave me something good. Don't judge me!
void Optimizer::optimizeSpacedPairs(std::vector<Instr> &firstPass) {
    Instr prev = {.var = {}, .type = InstrType::NONE};
    size_t prevIndex = 0;

    for (size_t i = 0; i < firstPass.size(); ++i) {
        Instr &instr = firstPass[i];

        if (instr.type == InstrType::Push) {
            PushInstr push = std::get<PushInstr>(instr.var);
            if (shouldIgnorePushPop(push.what)) continue;
            prev = instr;
            prevIndex = i;
        } else if (prev.type == InstrType::Push && instr.type == InstrType::Pop) {
            PopInstr pop = std::get<PopInstr>(instr.var);
            PushInstr push = std::get<PushInstr>(prev.var);

            // Only replace push/pop with mov if no intermediate instructions
            // are modifying the registers involved.
            bool canOptimize = true;

            // Check if the registers involved are used in any subsequent calculations
            for (size_t j = prevIndex + 1; j < i; ++j) {
                Instr &midInstr = firstPass[j];
                // If any register is used in a calculation, we shouldn't optimize
                if (midInstr.type == InstrType::Mov) {
                    MovInstr mov = std::get<MovInstr>(midInstr.var);
                    if (mov.dest == push.what || mov.src == push.what || 
                        mov.dest == pop.where || mov.src == pop.where) {
                        canOptimize = false;
                        break;
                    }
                }
            }

            if (canOptimize) {
                firstPass.erase(firstPass.begin() + prevIndex);
                firstPass.erase(firstPass.begin() + i - 1);

                Instr newInstr = {
                    .var = MovInstr{.dest = pop.where, .src = push.what, .destSize = pop.whereSize, .srcSize = push.whatSize},
                    .type = InstrType::Mov
                };
                firstPass.insert(firstPass.begin() + prevIndex, newInstr);

                // Skip over the new mov instruction and reset prev
                i = prevIndex;
                prev = Instr {.var = {}, .type = InstrType::NONE};
            } else {
                // If optimization isn't possible, just continue
                prev = instr;
            }
        }
    }
}



// Normal code compiled from user's zura will never affect the stack registers
// They will be affected when it is ABSOLUTELY necessary (for exanple, function scopes)
bool Optimizer::shouldIgnorePushPop(const std::string &reg) {
    return reg == "%rbp" || reg == "%rsp";
}