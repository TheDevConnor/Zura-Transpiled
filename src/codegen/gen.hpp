#pragma once

#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

// Global LLVM context and builder
static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static std::unique_ptr<llvm::Module> TheModule;

namespace codegen {
void initializeModule() {
  TheModule = std::make_unique<llvm::Module>("MyModule", TheContext);
}
} // namespace codegen