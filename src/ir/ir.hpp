#pragma once 

#include <vector>
#include <string>

class IRInstruction {
public:
    enum class Opcode {
        Add,
        Sub,
        Mul,
        Div,
        Load,
        Store,
        Jump,
        Branch,
        Call,
        Return
    };

    Opcode opcode;
    std::vector<IRInstruction*> operands;

    IRInstruction(Opcode op, const std::vector<IRInstruction*>& ops)
        : opcode(op), operands(ops) {}
};

class IRBasicBlock {
public:
    std::vector<IRInstruction*> instructions;
    
    void addInstruction(IRInstruction* instr) {
        instructions.push_back(instr);
    }
};

class IRFunction {
public:
    std::string name;
    std::vector<std::string> args;
    std::string returnType;
    std::vector<IRBasicBlock*> blocks;
    
    IRFunction(const std::string& funcName) : name(funcName) {}
    void addBlock(IRBasicBlock* block) {
        blocks.push_back(block);
    }
};
