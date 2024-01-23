#include "ast.hpp"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <map>
#include <memory>

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;
static std::map<std::string, Value *> NamedValues;

std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
Value *LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}

//! Expressions
Value *BinaryExprAST::codegen() {
  Value *right = this->RHS.get()->codegen();
  Value *left = this->LHS.get()->codegen();

  if (!left || !right)
    return nullptr;

  switch (this->Op) {
  case TokenKind::PLUS:
    return Builder->CreateAdd(left, right, "addtmp");
  case TokenKind::MINUS:
    return Builder->CreateSub(left, right, "subtmp");
  case TokenKind::STAR:
    return Builder->CreateMul(left, right, "multmp");
  case TokenKind::SLASH:
    return Builder->CreateSDiv(left, right, "divtmp");
  case TokenKind::EQUAL_EQUAL:
    return Builder->CreateICmpEQ(left, right, "eqtmp");
  case TokenKind::BANG_EQUAL:
    return Builder->CreateICmpNE(left, right, "neqtmp");
  case TokenKind::GREATER:
    return Builder->CreateICmpSGT(left, right, "gttmp");
  case TokenKind::GREATER_EQUAL:
    return Builder->CreateICmpSGE(left, right, "getmp");
  case TokenKind::LESS:
    return Builder->CreateICmpSLT(left, right, "lttmp");
  case TokenKind::LESS_EQUAL:
    return Builder->CreateICmpSLE(left, right, "letmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}
Value *LiteralExprAST::codegen() {
  switch (this->getNodeType()) {
  case AstNodeType::STRING_LITERAL:
    return Builder->CreateGlobalStringPtr(this->value.start);
  case AstNodeType::TRUE_LITERAL:
    return ConstantInt::get(*TheContext, APInt(1, 1));
  case AstNodeType::FALSE_LITERAL:
    return ConstantInt::get(*TheContext, APInt(1, 0));
  case AstNodeType::NIL_LITERAL:
    return ConstantPointerNull::get(
        PointerType::get(Type::getInt8Ty(*TheContext), 0));
  default:
    return LogErrorV("invalid literal");
  }

  return nullptr;
}
Value *NumberExprAST::codegen() {
  return ConstantInt::get(*TheContext, APInt(32, this->Val));
}
Value *UnaryExprAST::codegen() {
  Value *operand = this->RHS->codegen();
  if (!operand)
    return nullptr;

  switch (this->Op) {
  case TokenKind::MINUS:
    return Builder->CreateNeg(operand, "negtmp");
  case TokenKind::BANG:
    return Builder->CreateNot(operand, "nottmp");
  default:
    return LogErrorV("invalid unary operator");
  }
}
Value *GroupingExprAST::codegen() {
    return this->Expr->codegen();
}
Value *CallExprAST::codegen() {
    // Function *callee = TheModule->getFunction(this->Callee);
    // if (!callee)
    //     return LogErrorV("Unknown function referenced");
    
    // if (callee->arg_size() != this->Args.size())
    //     return LogErrorV("Incorrect # arguments passed");
    
    // std::vector<Value *> argsv;
    // for (unsigned i = 0, e = this->Args.size(); i != e; ++i) {
    //     argsv.push_back(this->Args[i]->codegen());
    //     if (!argsv.back())
    //     return nullptr;
    // }
    
    // return Builder->CreateCall(callee, argsv, "calltmp");
    return LogErrorV("CallExprAST::codegen() not implemented");
}
Value *IdentifierExprAST::codegen() {
    Value *V = NamedValues[this->Name];
    if (!V)
        return LogErrorV("Unknown variable name");
    return V;
}

//! Types
Value *TypeAST::codegen() {
    return nullptr;
}
Value *TypePointerAST::codegen() {
    return nullptr;
}
Value *TypeArrayAST::codegen() {
    return nullptr;
}

//! Arrays
Value *ArrayTypeAST::codegen() {
    return nullptr;
}
Value *ArrayExprAST::codegen() {
    return nullptr;
}

//! Statements
Value *ExpressionStmtAST::codegen() {
    return this->Expr->codegen();
}
Value *PrintStmtAST::codegen() {
    Value *V = this->Expr->codegen();
    if (!V)
        return nullptr;
    
    return Builder->CreateCall(TheModule->getFunction("print"), V, "printtmp");
}
Value *VarDeclStmtAST::codegen() {
    Value *V = NamedValues[this->Name.start];
    if (V)
        return LogErrorV("Variable with this name already declared");
    
    V = this->Expr->codegen();
    if (!V)
        return nullptr;

    NamedValues[this->Name.start] = V;
    return V;
}
Value *BlockStmtAST::codegen() {
    Value *V = nullptr;
    for (auto &stmt : this->Stmts) {
        V = stmt->codegen();
    }
    return V;
}
Value *ExitStmtAST::codegen() {
    return Builder->CreateCall(TheModule->getFunction("exit"), nullptr, "exittmp");
}
Value *ReturnStmtAST::codegen() {
    Value *V = this->Expr->codegen();
    if (!V)
        return nullptr;
    
    return Builder->CreateRet(V);
}

//! Functions
Value *FunctionDeclStmtAST::codegen() {
    std::vector<Type *> argTypes;
    for (auto &param : this->Params) {
        argTypes.push_back(Type::getInt32Ty(*TheContext));
    }
    
    FunctionType *FT = FunctionType::get(Type::getInt32Ty(*TheContext), argTypes, false);
    Function *F = Function::Create(FT, Function::ExternalLinkage, this->Name, TheModule.get());
    
    unsigned Idx = 0;
    for (auto &Arg : F->args()) {
        Arg.setName(this->Params[Idx++]);
    }
    
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", F);
    Builder->SetInsertPoint(BB);
    
    NamedValues.clear();
    for (auto &Arg : F->args()) {
        NamedValues[Arg.getName().str()] = &Arg;
    }
    
    if (Value *RetVal = this->Body->codegen()) {
        Builder->CreateRet(RetVal);
        verifyFunction(*F);
        return F;
    }
    
    F->eraseFromParent();
    return nullptr;
}

//! Program
Value *ProgramAST::codegen() {
    Value *V = nullptr;
    for (auto &stmt : this->Stmts) {
        V = stmt->codegen();
    }
    return V;
}