#include "../helper/error/error.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"
#include "gen.hpp"
#include <fstream>
#include <cstdint>


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

void codegen::processComparison(Node::Expr *cond, const std::string &jumpTrue, const std::string &jumpFalse, bool isLoop) {
  // Evaluate the 'cond' as a regular expression, but do not push the result to the stack and compare to 0:
  // just jump to the label if binary
  Node::Expr *simpleCond = CompileOptimizer::optimizeExpr(cond);
  if (simpleCond->kind == ND_BINARY) {
    // Check if its a BOOL operation
    BinaryExpr *expr = static_cast<BinaryExpr *>(simpleCond);
    if (isLoop) {
      JumpCondition jump = getJumpCondition(expr->op);
      visitExpr(expr->lhs);
      visitExpr(expr->rhs);
      popToRegister("%rax");
      popToRegister("%rbx");
      // Compare them LHS > RHS
      push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"}, .type = InstrType::Cmp}, Section::Main);
      // Jump if the condition is met
      push(Instr{.var=JumpInstr{.op=jump, .label=jumpTrue}, .type=InstrType::Jmp}, Section::Main);
      return;
    }
    if (boolOperations.find(expr->op) == boolOperations.end()) {
      // do the default
      visitExpr(simpleCond);
      PushInstr prevPush = std::get<PushInstr>(text_section[text_section.size() - 1].var);
      text_section.pop_back();
      pushLinker("testq " + prevPush.what + ", " + prevPush.what + "\n\t", Section::Main);
      if (jumpTrue != "")
        push(Instr{.var=JumpInstr{.op=JumpCondition::NotEqual, .label=jumpTrue}, .type=InstrType::Jmp}, Section::Main);
      else if (jumpFalse != "")
        push(Instr{.var=JumpInstr{.op=JumpCondition::Equal, .label=jumpFalse}, .type=InstrType::Jmp}, Section::Main);
      return;
    }
    // It does not matter which side we check for SymbolType, since they will both be enforced to be the same type
    bool isFloat = expr->lhs->asmType->kind == ND_SYMBOL_TYPE && (getUnderlying(expr->lhs->asmType) == "float");
    std::string lhsReg = isFloat ? "%xmm0" : "%rax";
    std::string rhsReg = isFloat ? "%xmm1" : "%rbx";
    if (getExpressionDepth(expr->lhs) > getExpressionDepth(expr->rhs)) {
      visitExpr(expr->lhs);
      visitExpr(expr->rhs);
      popToRegister(rhsReg);
      popToRegister(lhsReg);
    } else {
      visitExpr(expr->rhs);
      visitExpr(expr->lhs);
      popToRegister(lhsReg);
      popToRegister(rhsReg);
    }
    if (isFloat) {
      pushLinker("ucomiss %xmm1, %xmm0\n\t", Section::Main);
    } else {
      push(Instr{.var = CmpInstr{.lhs = lhsReg, .rhs = rhsReg}, .type = InstrType::Cmp}, Section::Main);
    }
    if (jumpTrue != "")
      push(Instr{.var=JumpInstr{.op=getJumpCondition(expr->op), .label=jumpTrue}, .type=InstrType::Jmp}, Section::Main);
    else if (jumpFalse != "")
      push(Instr{.var=JumpInstr{.op=getOpposite(getJumpCondition(expr->op)), .label=jumpFalse}, .type=InstrType::Jmp}, Section::Main);
    return; // the good ending 
  }
  // check if the regular expression is 0
  visitExpr(simpleCond);
  PushInstr prevPush = std::get<PushInstr>(text_section[text_section.size() - 1].var);
  text_section.pop_back();
  pushLinker("testq " + prevPush.what + ", " + prevPush.what + "\n\t", Section::Main);
  if (jumpTrue != "")
    push(Instr{.var=JumpInstr{.op=JumpCondition::NotEqual, .label=jumpTrue}, .type=InstrType::Jmp}, Section::Main);
  else if (jumpFalse != "")
    push(Instr{.var=JumpInstr{.op=JumpCondition::Equal, .label=jumpFalse}, .type=InstrType::Jmp}, Section::Main);
  // sadge
  return;
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

void codegen::handleStrType(Node::Expr *arg) {
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;
  visitExpr(arg);
  popToRegister("%rsi"); // String address
  moveRegister("%rdi", "%rsi", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call},
       Section::Main);
  moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of string
  prepareSyscallWrite();
}

void codegen::handlePrimType(Node::Expr *arg) {
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;
  nativeFunctionsUsed[NativeASMFunc::itoa] = true;
  visitExpr(arg);
  popToRegister("%rdi");
  push(Instr{.var = CallInstr{.name = "native_itoa"}, .type = InstrType::Call},
       Section::Main); // Convert int to string
  moveRegister("%rdi", "%rax", DataSize::Qword, DataSize::Qword);
  moveRegister("%rsi", "%rdi", DataSize::Qword, DataSize::Qword);
  // Now we have the integer string in %rax (assuming %rax holds the pointer to the result)
  push(Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call},
       Section::Main);
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