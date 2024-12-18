#include "../helper/error/error.hpp"
#include "gen.hpp"
#include "optimizer/instr.hpp"
#include <fstream>


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

void codegen::processBinaryExpression(BinaryExpr *cond,
                                      const std::string &preconCount,
                                      const std::string &name, bool isLoop) {
  // TODO: Float comparisons
  // Evaluate LHS first, store in %rax
  visitExpr(cond->lhs);
  popToRegister("%rax");

  // Evaluate RHS, store in %rbx
  visitExpr(cond->rhs);
  popToRegister("%rbx");

  if (isLoop) {
    // Perform comparison (this order ensures LHS > RHS works correctly)(loops)
    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"},
               .type = InstrType::Cmp},
         Section::Main);

    // Jump based on the correct condition
    JumpCondition jmpCond = getJumpCondition(cond->op);

    // Recerse the condition for loops (e.g., for i = 0; i < 10; i++) to jump if i >= 10
    switch (jmpCond) {
      case JumpCondition::Less:
        jmpCond = JumpCondition::GreaterEqual;
        break;
      case JumpCondition::LessEqual:
        jmpCond = JumpCondition::Greater;
        break;
      case JumpCondition::Greater:
        jmpCond = JumpCondition::LessEqual;
        break;
      case JumpCondition::GreaterEqual:
        jmpCond = JumpCondition::Less;
        break;
      case JumpCondition::NotEqual:
        jmpCond = JumpCondition::Equal;
        break;
      case JumpCondition::Equal:
        jmpCond = JumpCondition::NotEqual;
        break;
      default:
        break;
      }

    push(Instr{.var = JumpInstr{.op = jmpCond, .label = name },
               .type = InstrType::Jmp},
         Section::Main);
    return;
  } else {
    // Perform comparison (this order ensures LHS > RHS works correctly)(ifs)
    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = "%rbx"},
               .type = InstrType::Cmp},
         Section::Main);
  }

  // Jump based on the correct condition
  JumpCondition jmpCond = getJumpCondition(cond->op);
  push(Instr{.var = JumpInstr{.op = jmpCond, .label = name + preconCount},
             .type = InstrType::Jmp},
       Section::Main);
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
  }
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
    // Get the size of the underlying type
    // multiply by # of elements
    ArrayType *arr = static_cast<ArrayType *>(type);
    if (arr->constSize < 1) return getByteSizeOfType(arr->underlying);
    return getByteSizeOfType(arr->underlying) * arr->constSize; 
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