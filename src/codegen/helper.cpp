#include "../helper/error/error.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"
#include "gen.hpp"
#include <fstream>
#include <cmath>


void codegen::handleError(int line, int pos, std::string msg,
                           std::string typeOfError, bool isFatal) {
  Lexer lexer; // dummy lexer
  ErrorClass::error(line, pos, msg, "", typeOfError, node.current_file, lexer,
                    node.tks, false, false, isFatal,
                    false, false, true);
}

/*
 * @brief Appends a pushq instruction to the text section, using {reg} as the register to push
 * 
 * Also look at {@link popToRegister}
 * @param reg The register to push
*/
void codegen::pushRegister(const std::string &reg) {
  push(Instr{.var = PushInstr{.what = reg}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

/*
 * @brief Appends a popq instruction to the text section, using {reg} as the register to pop to
 * 
 * Also look at {@link pushRegister}
 * @param reg The register to pop to
*/
void codegen::popToRegister(const std::string &reg) {
  push(Instr{.var = PopInstr{.where = reg}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;
}

void codegen::moveRegister(const std::string &dest, const std::string &src,
                           DataSize dest_size, DataSize src_size) {
  push(Instr{.var = MovInstr{.dest = dest,
                             .src = src,
                             .destSize = dest_size,
                             .srcSize = src_size},
             .type = InstrType::Mov},
       Section::Main);
}

void codegen::pushLinker(std::string val, Section section) {
  push(Instr {
    .var = LinkerDirective {
      .value = val
    },
    .type = InstrType::Linker
  }, section);
}
JumpCondition codegen::getOpposite(JumpCondition in) {
  switch (in) {
    case JumpCondition::Equal:
      return JumpCondition::NotEqual;
    case JumpCondition::NotEqual:
      return JumpCondition::Equal;
    case JumpCondition::Greater:
      return JumpCondition::LessEqual;
    case JumpCondition::GreaterEqual:
      return JumpCondition::Less;
    case JumpCondition::Less:
      return JumpCondition::GreaterEqual;
    case JumpCondition::LessEqual:
      return JumpCondition::Greater;
    case JumpCondition::Unconditioned:
    default:
      return JumpCondition::Unconditioned;
  }
};

JumpCondition codegen::getJumpCondition(const std::string &op) {
  if (op == ">")
    return JumpCondition::Greater;
  if (op == ">=")
    return JumpCondition::GreaterEqual;
  if (op == "<")
    return JumpCondition::Less;
  if (op == "<=")
    return JumpCondition::LessEqual;
  if (op == "==")
    return JumpCondition::Equal;
  if (op == "!=")
    return JumpCondition::NotEqual;
  std::cerr << "Invalid operator for comparison" << std::endl;
  exit(-1);
}

int codegen::getExpressionDepth(Node::Expr *e) {
  if (e->kind != NodeKind::ND_BINARY) return 1; 
  BinaryExpr *expr = static_cast<BinaryExpr *>(e);
  int lhsDepth = 1;
  if (expr->lhs->kind == NodeKind::ND_BINARY) {
    lhsDepth += getExpressionDepth(static_cast<BinaryExpr *>(expr->lhs));
  } else if (expr->lhs->kind == NodeKind::ND_GROUP) {
    GroupExpr *group = static_cast<GroupExpr *>(expr->lhs);
    if (group->expr->kind == NodeKind::ND_BINARY) {
      lhsDepth += getExpressionDepth(static_cast<BinaryExpr *>(group->expr));
    }
  }
  // check for rhs and see if its greater
  int rhsDepth = 1;
  if (expr->rhs->kind == NodeKind::ND_BINARY) {
    rhsDepth += getExpressionDepth(static_cast<BinaryExpr *>(expr->rhs));
  } else if (expr->rhs->kind == NodeKind::ND_GROUP) {
    GroupExpr *group = static_cast<GroupExpr *>(expr->rhs);
    if (group->expr->kind == NodeKind::ND_BINARY) {
      rhsDepth += getExpressionDepth(static_cast<BinaryExpr *>(group->expr));
    }
  }
  return lhsDepth > rhsDepth ? lhsDepth : rhsDepth;
}

JumpCondition codegen::processComparison(Node::Expr *cond) {
  // Evaluate the expression. Return the jump of condition of when the comparison is TRUE
  Node::Expr *eval = CompileOptimizer::optimizeExpr(cond);
  if (cond->kind == ND_BINARY) {
    BinaryExpr *bin = static_cast<BinaryExpr *>(eval);
    if (boolOperations.find(bin->op) != boolOperations.end()) {
      // we can do a little bit of optimizing here
      int lhsDepth = getExpressionDepth(bin->lhs);
      int rhsDepth = getExpressionDepth(bin->rhs);
      bool isFloat = bin->asmType->kind == ND_SYMBOL_TYPE && getUnderlying(bin->asmType) == "float";
      std::string lhsReg = isFloat ? "%xmm0" : "%rax";
      std::string rhsReg = isFloat ? "%xmm1" : "%rbx";
      if (lhsDepth > rhsDepth) {
        visitExpr(bin->lhs);
        visitExpr(bin->rhs);
        popToRegister(rhsReg);
        popToRegister(lhsReg);
      } else {
        visitExpr(bin->rhs);
        visitExpr(bin->lhs);
        popToRegister(lhsReg);
        popToRegister(rhsReg);
      }
      // perform the compare
      if (isFloat) {
        pushLinker("ucomiss %xmm1, %xmm0\n\t", Section::Main);  
      } else {
        push(Instr{.var = CmpInstr{.lhs = lhsReg, .rhs = rhsReg}, .type = InstrType::Cmp}, Section::Main);
      }
      return getJumpCondition(bin->op); 
    }
  }
  visitExpr(eval);
  PushInstr instr = std::get<PushInstr>(text_section[text_section.size() - 1].var);
  text_section.pop_back();
  pushLinker("test " + instr.what + ", " + instr.what + "\n\t", Section::Main);
  return JumpCondition::NotZero;
}

void codegen::handleExitSyscall() {
  moveRegister("%rax", "$60", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_EXIT"}, .type = InstrType::Syscall},
       Section::Main);
}

void codegen::handleReturnCleanup() {
  popToRegister("%rbp");
  push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
}

int codegen::convertFloatToInt(float input) {
  union {
    float f;
    int i;
  } u;
  u.f = input;
  return u.i;
}

bool codegen::execute_command(const std::string &command,
                              const std::string &log_file) {
  std::string command_with_error_logging = command + " 2> " + log_file;
  int result = std::system(command_with_error_logging.c_str());
  // Read log file
  std::ifstream log(log_file);
  std::string log_contents = "";

  if (!log.is_open()) {
    handleError(0, 0, "Error opening log file: " + log_file, "Codegen Error");
    return false;
  }
  
  std::string line;
  bool first = true;
  while (getline(log, line)) {
    if (first) {
      log_contents += line;
      first = false;
    } else {
      log_contents += "\n\t" + line;
    }
  }
  
  log.close();

  if (result != 0) {
    handleError(0, 0,
                 "Error executing command: " + command + "\n\t" + log_contents,
                 "Codegen Error");
    return false;
  }
  return true;
}

void codegen::push(Instr instr, Section section) {
  switch (section) {
    case Section::Main:
      text_section.push_back(instr);
      break;
    case Section::Head:
      head_section.push_back(instr);
      break;
    case Section::Data:
      data_section.push_back(instr);
      break;
    case Section::ReadonlyData:
      rodt_section.push_back(instr);
      break;
    case Section::DIE:
      die_section.push_back(instr);
      break;
    case Section::DIEString:
      dies_section.push_back(instr);
      break;
    case Section::DIEAbbrev:
      diea_section.push_back(instr);
      break;
    case Section::DIETypes: {
      diet_section.push_back(instr);
      break;
    }
  }
}

/*
  * Helper functions for printing to the console
*/
void codegen::prepareSyscallWrite() {
  // syscall id for write on x86 is 1
  moveRegister("%rax", "$1", DataSize::Qword, DataSize::Qword);
  // set rdi to 1 (file descriptor for stdout)
  moveRegister("%rdi", "$1", DataSize::Qword, DataSize::Qword);
  // Make syscall to write
  push(Instr{.var = Syscall{.name = "SYS_WRITE"}, .type = InstrType::Syscall},
       Section::Main);
}

void codegen::handlePtrType(Node::Expr *arg, PrintStmt *print) {
  if (getUnderlying(arg->asmType).find("char") == 1) {
    visitExpr(arg);
    popToRegister("%rsi");
    prepareSyscallWrite();
  } else {
    handleError(print->line, print->pos,
                "Cannot print pointer type. Dereference first or print as an address (int cast).",
                "Codegen Error");
  }
}

void codegen::handleLiteral(Node::Expr *arg) {
  // This function effectively just prints the value of the literal
  std::string strLabel = "string" + std::to_string(stringCount++);
  std::string stringValue;
  if (arg->kind == ND_INT) {
    IntExpr *intExpr = static_cast<IntExpr *>(arg);
    stringValue = std::to_string(intExpr->value);
    push(Instr{.var=Comment{.comment="Print integer literal for some reason"},.type=InstrType::Comment},Section::Main);
  };
  if (arg->kind == ND_BOOL) {
    BoolExpr *boolExpr = static_cast<BoolExpr *>(arg);
    stringValue = boolExpr->value ? "true" : "false";
    push(Instr{.var=Comment{.comment="Print boolean literal for some reason"},.type=InstrType::Comment},Section::Main);
  };
  if (arg->kind == ND_FLOAT) {
    FloatExpr *floatExpr = static_cast<FloatExpr *>(arg);
    stringValue = std::to_string(floatExpr->value);
    push(Instr{.var=Comment{.comment="Print float literal for some reason"},.type=InstrType::Comment},Section::Main);
  }
  // Char expr is the same as an intexpr and its kinda unused on its own

  // Define the string in the readonly data section
  push(Instr{.var=Label{.name=strLabel},.type=InstrType::Label},Section::ReadonlyData);
  //? This shouldnt be an asciz because we know the length of the str ahead of time but im not implementing the .str/.ascii directive
  push(Instr{.var=AscizInstr{.what=stringValue},.type=InstrType::Asciz},Section::ReadonlyData);
  // perform the operation by moving the string into %rsi and the length into %rdx
  moveRegister("%rsi", strLabel + "(%rip)", DataSize::Qword, DataSize::Qword);
  moveRegister("%rdx", "$" + std::to_string(stringValue.length()), DataSize::Qword, DataSize::Qword);
  prepareSyscallWrite(); // The end!
};

void codegen::handleStrType(Node::Expr *arg) {
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;
  visitExpr(arg);
  popToRegister("%rsi"); // String address
  moveRegister("%rdi", "%rsi", DataSize::Qword, DataSize::Qword);
  // calls might mess up any variables on the stack
  // lets make sure that does not happen by subtracting from stack
  push(Instr{.var=SubInstr{.lhs="%rsp",.rhs="$"+std::to_string(variableCount)},.type=InstrType::Sub},Section::Main);
  push(Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call},
       Section::Main);
  push(Instr{.var=AddInstr{.lhs="%rsp",.rhs="$"+std::to_string(variableCount)},.type=InstrType::Sub},Section::Main);
  moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of string
  prepareSyscallWrite();
}

void codegen::handlePrimType(Node::Expr *arg) {
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;
  nativeFunctionsUsed[NativeASMFunc::itoa] = true;
  visitExpr(arg);
  popToRegister("%rdi");
  // subtract rsp
  push(Instr{.var=SubInstr{.lhs="%rsp",.rhs="$"+std::to_string(variableCount)},.type=InstrType::Sub},Section::Main);
  push(Instr{.var = CallInstr{.name = "native_itoa"}, .type = InstrType::Call},
       Section::Main); // Convert int to string
  moveRegister("%rdi", "%rax", DataSize::Qword, DataSize::Qword);
  moveRegister("%rsi", "%rdi", DataSize::Qword, DataSize::Qword);
  // Now we have the integer string in %rax (assuming %rax holds the pointer to the result)
  push(Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call},
       Section::Main);
  push(Instr{.var=AddInstr{.lhs="%rsp",.rhs="$"+std::to_string(variableCount)},.type=InstrType::Sub},Section::Main);
  moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of number string
  prepareSyscallWrite();
}

void codegen::handleFloatType(Node::Expr *arg) {
    std::string msg = "Printing floats is not supported yet.";
    handleError(0, 0, msg, "Codegen Error");
}

// Add 1
int codegen::getFileID(const std::string &file) {
  for (int i = 0; i < fileIDs.size(); i++) {
    if (fileIDs[i] == file) {
      return i;
    }
  }
  fileIDs.push_back(file);
  return fileIDs.size() - 1;
}

void codegen::pushDebug(int line, int file, int column) {
  // If not in debug mode, this funciton will pretty much be a massive nop.
  if (!debug) return;
  if (column != -1) {
    push(Instr{.var = LinkerDirective{.value = ".loc " + std::to_string(file) + " " +
                                                std::to_string(line) + " " + std::to_string(column) + "\n\t"},
                .type = InstrType::Linker},
          Section::Main);
  } else {
    push(Instr{.var = LinkerDirective{.value = ".loc " + std::to_string(file) + " " +
                                                std::to_string(line) + "\n\t"},
                .type = InstrType::Linker},
          Section::Main);
  }
}

// A real type, not the asmType
signed short int codegen::getByteSizeOfType(Node::Type *type) {
  if (type->kind == NodeKind::ND_POINTER_TYPE) {
    return 8;
  }
  if (type->kind == NodeKind::ND_ARRAY_TYPE) {
    return 8; // Pointer to first element
  }
  if (type->kind == NodeKind::ND_SYMBOL_TYPE) {
    SymbolType *sym = static_cast<SymbolType *>(type);
    if (sym->name == "int") {
      return 8;
    } else if (sym->name == "char") {
      return 1;
    } else if (sym->name == "float") {
      return 8;
    } else if (sym->name == "void") {
      return 0;
    } else if (structByteSizes.find(sym->name) != structByteSizes.end()) {
      return structByteSizes[sym->name].first;
    } else {
      return 8; // Default to 8 bytes because x64 rules!!
    }
    std::cout << "Unknown type: " << sym->name << std::endl;
    return -1;
  }
  // Unreachable!
  std::cout << "Reached unknown place in the getByteSizeOfType ..." << std::endl;
  return -1;
};

std::string codegen::getUnderlying(Node::Type *type) {
  // Eventually, all underlying's will turn into SymbolType's, which is just the name of the type.
  if (type->kind == ND_POINTER_TYPE) {
    PointerType *p = static_cast<PointerType *>(type);
    return getUnderlying(p->underlying);
  }
  if (type->kind == ND_ARRAY_TYPE) {
    ArrayType *a = static_cast<ArrayType *>(type);
    return getUnderlying(a->underlying);
  }
  if (type->kind == ND_SYMBOL_TYPE) {
    SymbolType *s = static_cast<SymbolType *>(type);
    return s->name; // The end!
  }
  if (type->kind == ND_TEMPLATE_STRUCT_TYPE) {
    TemplateStructType *t = static_cast<TemplateStructType *>(type);
    return getUnderlying(t->underlying);
  }
  // Unreachable!
  std::cout << "Unreachable code ... well, reached!" << std::endl;
  std::cout << std::to_string((int)type->kind) << std::endl;
  return "";
};

std::string codegen::type_to_diename(Node::Type *type) {
  // just imagine what a [[int *]*] would look like
  // int_ptr_arr_ptr_arr LMAO
  if (type->kind == ND_POINTER_TYPE) {
    PointerType *p = static_cast<PointerType *>(type);
    return type_to_diename(p->underlying) + "_ptr";
  }
  if (type->kind == ND_ARRAY_TYPE) {
    ArrayType *a = static_cast<ArrayType *>(type);
    if (a->constSize <= 0) {
      // Variable-length array.
      return type_to_diename(a->underlying) + "_arr";
    }
    return type_to_diename(a->underlying) + "_arr" + std::to_string(a->constSize - 1);
  }
  if (type->kind == ND_SYMBOL_TYPE) { // Yes, structs are symbols too
    SymbolType *s = static_cast<SymbolType *>(type);
    return s->name;
  }
  // Unreachable!
  return "";
};

// This will usually be a multiple of 8 because x64 rules!
// Multiple: 8
// 16 -> 16
// 17 -> 24
int codegen::round(int num, int multiple) {
  return (num + multiple - 1) / multiple * multiple;
};

// Get the number of bytes that `value` takes up when encoded in the LEB128 format.
size_t codegen::sizeOfLEB(int64_t value) {
    size_t size = 0;
    bool more = true;

    while (more) {
        uint8_t byte = value & 0x7F; // Get the lowest 7 bits 
        value >>= 7;                // Arithmetic right shift for signed values

        // Determine if more bytes are needed
        if ((value == 0 && (byte & 0x40) == 0) || (value == -1 && (byte & 0x40) != 0)) {
            more = false; // Terminate if no more bytes are needed
        }

        ++size; // Count the byte
    }

    return size;
}