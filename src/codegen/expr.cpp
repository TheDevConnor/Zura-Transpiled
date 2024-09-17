#include "gen.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>

llvm::Value *IntExpr::codegen() const {
  return llvm::ConstantInt::get(TheContext, llvm::APInt(32, value));
}

llvm::Value *FloatExpr::codegen() const {
  return llvm::ConstantFP::get(TheContext, llvm::APFloat(value));
}

llvm::Value *BoolExpr::codegen() const {
  return llvm::ConstantInt::get(TheContext, llvm::APInt(1, value ? 1 : 0));
}

llvm::Value *StringExpr::codegen() const {
  return nullptr;
}

llvm::Value *BinaryExpr::codegen() const {
  llvm::Value *L = lhs->codegen();
  llvm::Value *R = rhs->codegen();

  // verify that they are not null
  if (!L || !R)
    return nullptr; // TODO: Handle this error later

  // We need to see if it is a float or an int
  bool isLFloat = L->getType()->isFloatingPointTy();
  bool isRFloat = R->getType()->isFloatingPointTy();

  if (isLFloat && !isRFloat) {
    R = Builder.CreateSIToFP(R, llvm::Type::getFloatTy(TheContext),
                             "intToFloatR");
  } else if (!isLFloat && isRFloat) {
    L = Builder.CreateSIToFP(L, llvm::Type::getFloatTy(TheContext),
                             "intToFloatL");
  }

  bool isFloat = isLFloat || isRFloat;

  switch (op[0]) {
  case '+':
    return isFloat ? Builder.CreateFAdd(L, R, "faddtmp")
                   : Builder.CreateAdd(L, R, "addtmp");
  case '-':
    return isFloat ? Builder.CreateFSub(L, R, "fsubtmp")
                   : Builder.CreateSub(L, R, "suntmp");
  case '*':
    return isFloat ? Builder.CreateFMul(L, R, "fmultmp")
                   : Builder.CreateMul(L, R, "multmp");
  case '/':
    return isFloat ? Builder.CreateFDiv(L, R, "fdivtmp")
                   : Builder.CreateSDiv(L, R, "divtmp");
  default:
    std::cerr << "Unknown binary operator: " << op << "\n";
    return nullptr;
  }
}