#include "../helper/error/error.hpp"
#include "../typeChecker/type.hpp"
#include "gen.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <unordered_map>

void codegen::handleError(int line, int pos, std::string msg,
                          std::string typeOfError, bool isFatal) {
  (void)isFatal;
  Error::handle_error(typeOfError, node.current_file, msg, node.tks, line, pos, pos + 1);
}

/*
 * @brief Appends a pushq instruction to the text section, using {reg} as the
 * register to push
 *
 * Also look at {@link popToRegister}
 * @param reg The register to push
 */
void codegen::pushRegister(const std::string &reg, DataSize regSize) {
  push(Instr{.var = PushInstr{.what = reg, .whatSize = regSize}, .type = InstrType::Push},
       Section::Main);
  stackSize++;
}

/*
 * @brief Appends a popq instruction to the text section, using {reg} as the
 * register to pop to
 *
 * Also look at {@link pushRegister}
 * @param reg The register to pop to
 */
void codegen::popToRegister(const std::string &reg, DataSize regSize) {
  push(Instr{.var = PopInstr{.where = reg, .whereSize = regSize}, .type = InstrType::Pop},
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
  push(Instr{.var = LinkerDirective{.value = val}, .type = InstrType::Linker},
       section);
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
  case JumpCondition::Zero:
    return JumpCondition::NotZero;
  case JumpCondition::NotZero:
    return JumpCondition::Zero;
  case JumpCondition::NotGreater:
    return JumpCondition::Greater;
  case JumpCondition::NotLess:
    return JumpCondition::Less;
  default:
  case JumpCondition::Unconditioned:
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
  if (e->kind != NodeKind::ND_BINARY)
    return 1;
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
  // Evaluate the expression. Return the jump of condition of when the
  // comparison is TRUE
  Node::Expr *eval = CompileOptimizer::optimizeExpr(cond);
  if (cond->kind == ND_BINARY) {
    BinaryExpr *bin = static_cast<BinaryExpr *>(eval);
    if (boolOperations.find(bin->op) != boolOperations.end()) {
      // we can do a little bit of optimizing here
      int lhsDepth = getExpressionDepth(bin->lhs);
      int rhsDepth = getExpressionDepth(bin->rhs);
      bool isFloat = bin->lhs->asmType->kind == ND_SYMBOL_TYPE &&
                     getUnderlying(bin->lhs->asmType) == "float";
      bool isDouble = bin->lhs->asmType->kind == ND_SYMBOL_TYPE &&
                      getUnderlying(bin->lhs->asmType) == "double";
      DataSize size = DataSize::Qword;
      std::string lhsReg = "%rax";
      std::string rhsReg = "%rbx";
      switch (std::max(getByteSizeOfType(bin->lhs->asmType),
                       getByteSizeOfType(bin->rhs->asmType))) {
      case 1:
        lhsReg = "%al";
        rhsReg = "%bl";
        size = DataSize::Byte;
        break;
      case 2:
        lhsReg = "%ax";
        rhsReg = "%bx";
        size = DataSize::Word;
        break;
      case 4:
        lhsReg = "%eax";
        rhsReg = "%ebx";
        size = DataSize::Dword;
        break;
      case 8:
      default:
        break; // It's already OK
      }
      handleBinaryExprs(lhsDepth, rhsDepth, lhsReg, rhsReg, bin->lhs, bin->rhs);
      // perform the compare
      if (isFloat || isDouble) {
        pushLinker("ucomiss %xmm1, %xmm0\n\t", Section::Main);
      } else {
        push(Instr{.var = CmpInstr{.lhs = lhsReg, .rhs = rhsReg, .size = size},
                   .type = InstrType::Cmp},
             Section::Main);
      }
      return getJumpCondition(bin->op);
    }
  }
  visitExpr(eval);
  popToRegister("%rax");
  pushLinker("testq %rax, %rax\n\t", Section::Main);
  return JumpCondition::NotZero; // If it is not zero, it is rue
}

void codegen::handleBinaryExprs(int lhsDepth, int rhsDepth, std::string lhsReg,
                                std::string rhsReg, Node::Expr *lhs,
                                Node::Expr *rhs) {
  // you cant add an int to a float so they will both be float here
  bool isFloat = lhs->asmType->kind == ND_SYMBOL_TYPE &&
                 (getUnderlying(lhs->asmType) == "float" || 
                  getUnderlying(lhs->asmType) == "double");
  DataSize lhsSize = intDataToSize(getByteSizeOfType(lhs->asmType));
  DataSize rhsSize = intDataToSize(getByteSizeOfType(rhs->asmType));
  if (isFloat) {
    lhsSize = intDataToSizeFloat(getByteSizeOfType(lhs->asmType));
    rhsSize = intDataToSizeFloat(getByteSizeOfType(rhs->asmType));
  }
  // if (lhsDepth <= 1 && rhsDepth <= 1) {
  //   // If both are depth 1, we can just load them into registers
  //   visitExpr(lhs);
  //   popToRegister(lhsReg, lhsSize);
  //   visitExpr(rhs);
  //   popToRegister(rhsReg, rhsSize);
  //   return;
  // }
  if (lhsDepth > rhsDepth) {
    visitExpr(lhs);
    push(Instr{.var = PopInstr{.where = std::to_string(-variableCount) + "(%rbp)", .whereSize = lhsSize}, .type = InstrType::Pop}, Section::Main);
    visitExpr(rhs);
    push(Instr{.var = PopInstr{.where = rhsReg, .whereSize = lhsSize}, .type = InstrType::Pop}, Section::Main);
    moveRegister(lhsReg, std::to_string(-variableCount) + "(%rbp)", lhsSize, lhsSize);
  } else {
    visitExpr(rhs);
    push(Instr{.var = PopInstr{.where = std::to_string(-variableCount) + "(%rbp)", .whereSize = rhsSize}, .type = InstrType::Pop}, Section::Main);
    visitExpr(lhs);
    push(Instr{.var = PopInstr{.where = lhsReg, .whereSize = lhsSize}, .type = InstrType::Pop}, Section::Main);
    moveRegister(rhsReg, std::to_string(-variableCount) + "(%rbp)", rhsSize, rhsSize);
  }
}

void codegen::handleExitSyscall() {
  moveRegister("%rax", "$60", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_EXIT"}, .type = InstrType::Syscall},
       Section::Main);
}

void codegen::handleReturnCleanup() {
  popToRegister("%rbp");
  pushLinker(".cfi_def_cfa %rsp, 8\n\t", Section::Main);
  push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
}

size_t codegen::convertFloatToInt(std::string input) {
  union {
    double f; // 64-bit float
    size_t i; // 64-bit int
  } u;
  u.f = std::stod(input);
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
    std::string error_message = "Error executing command: " + command + "\n";
    if (log_contents.size() > 0) {
      error_message += "\t" + log_contents;
    }
    handleError(0, 0, error_message, "Codegen Error", true);
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
  // Make syscall to write
  push(Instr{.var = Syscall{.name = "SYS_WRITE"}, .type = InstrType::Syscall},
       Section::Main);
}

void codegen::handlePtrDisplay(Node::Expr *fd, Node::Expr *arg, int line,
                               int pos) {
  if (getUnderlying(arg->asmType) == "char") {
    // Printing a char*
    visitExpr(arg);
    popToRegister("%rsi");
    visitExpr(fd);
    popToRegister("%rdi");
    prepareSyscallWrite();
  } else {
    handleError(line, pos,
                "Cannot print pointer type. Dereference first or print as an "
                "address (int cast).",
                "Codegen Error");
  }
}

void codegen::handleArrayDisplay(Node::Expr *fd, Node::Expr *arg, int line,
                                 int pos) {
  if (getUnderlying(arg->asmType) != "char") {
    handleError(line, pos,
                "Cannot print array of type '" + getUnderlying(arg->asmType) +
                    "'. Only []char is supported.",
                "Codegen Error");
    return; // Refuse to print
  }

  // this function will only ever happen in a []char and never anywhere else
  ArrayType *at = static_cast<ArrayType *>(arg->asmType);
  visitExpr(arg);        // the lea
  popToRegister("%rsi"); // syscall arg
  moveRegister(
      "%rdx",
      "$" + std::to_string(getByteSizeOfType(at->underlying) * at->constSize),
      DataSize::Qword, DataSize::Qword); // byte count
  visitExpr(fd);
  popToRegister("%rdi");
  prepareSyscallWrite();
}

void codegen::handleLiteralDisplay(Node::Expr *fd, Node::Expr *arg) {
  // This function effectively just prints the value of the literal
  std::string strLabel = "string" + std::to_string(stringCount++);
  std::string stringValue;
  if (arg->kind == ND_INT) {
    IntExpr *intExpr = static_cast<IntExpr *>(arg);
    stringValue = std::to_string(intExpr->value);
    push(Instr{.var =
                   Comment{.comment = "Print integer literal for some reason"},
               .type = InstrType::Comment},
         Section::Main);
  };
  if (arg->kind == ND_BOOL) {
    BoolExpr *boolExpr = static_cast<BoolExpr *>(arg);
    stringValue = boolExpr->value ? "true" : "false";
    push(Instr{.var =
                   Comment{.comment = "Print boolean literal for some reason"},
               .type = InstrType::Comment},
         Section::Main);
  };
  if (arg->kind == ND_FLOAT) {
    FloatExpr *floatExpr = static_cast<FloatExpr *>(arg);
    stringValue = floatExpr->value;
    push(Instr{.var = Comment{.comment = "Print float literal for some reason"},
               .type = InstrType::Comment},
         Section::Main);
  };
  if (arg->kind == ND_STRING) {
    // If the string literal was a newline character, it was likely emitted
    // by outputln. This fills the rodata section with repetitive garbage.
    StringExpr *stringExpr = static_cast<StringExpr *>(arg);
    stringValue = stringExpr->value.substr(
      1,
      stringExpr->value.size() - 2); // Quotes are included, for some reason.
    if (stringValue == "\\n") {
      stringCount--; // Shhhh
      isUsingNewline = true;
      push(Instr{.var=LeaInstr{.size=DataSize::Qword, .dest = "%rsi", .src = ".Lstring_newline(%rip)"}, .type=InstrType::Lea}, Section::Main);
      moveRegister("%rdx", "$1", DataSize::Qword, DataSize::Qword);
      visitExpr(fd);
      popToRegister("%rdi");
      prepareSyscallWrite(); // The end!
      return;
    }
    push(
        Instr{.var = Comment{.comment = "Print string literal for some reason"},
              .type = InstrType::Comment},
        Section::Main);
  }
  // Char expr is the same as an intexpr and its kinda unused on its own

  // Define the string in the readonly data section
  push(Instr{.var = Label{.name = strLabel}, .type = InstrType::Label},
       Section::ReadonlyData);
  //? This shouldnt be an asciz because we know the length of the str ahead of
  //time but im not implementing the .str/.ascii directive
  push(Instr{.var = AscizInstr{.what = '"' + stringValue + '"'},
             .type = InstrType::Asciz},
       Section::ReadonlyData);
  // perform the operation by moving the string into %rsi and the length into
  // %rdx
  push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                             .dest = "%rsi",
                             .src = strLabel + "(%rip)"},
             .type = InstrType::Lea},
       Section::Main);

  // if any of the characters in the string are \ and then n,r,t, etc, or
  // something like that, we must subtract
  size_t realsize = stringValue.size();
  for (size_t i = 0; i < stringValue.size(); i++) {
    if (stringValue[i] == '\\') {
      realsize--;
    }
  }
  moveRegister("%rdx", "$" + std::to_string(realsize), DataSize::Qword,
               DataSize::Qword);
  visitExpr(fd);
  popToRegister("%rdi");
  prepareSyscallWrite(); // The end!
};

void codegen::handleStrDisplay(Node::Expr *fd, Node::Expr *arg) {
  nativeFunctionsUsed[NativeASMFunc::strlen_func] = true;
  visitExpr(arg);
  popToRegister("%rsi"); // String address
  // calls might mess up any variables on the stack
  // lets make sure that does not happen by subtracting from stack
  push(Instr{.var = SubInstr{.lhs = "%rsp",
                             .rhs = "$" + std::to_string(variableCount),
                             .size = DataSize::Qword},
             .type = InstrType::Sub},
       Section::Main);
  moveRegister("%rdi", "%rsi", DataSize::Qword, DataSize::Qword);
  push(
      Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call},
      Section::Main);
  push(Instr{.var = AddInstr{.lhs = "%rsp",
                             .rhs = "$" + std::to_string(variableCount),
                             .size = DataSize::Qword},
             .type = InstrType::Sub},
       Section::Main);
  moveRegister("%rdx", "%rax", DataSize::Qword,
               DataSize::Qword); // Length of string
  visitExpr(fd);
  popToRegister("%rdi");
  prepareSyscallWrite();
}

void codegen::handlePrimitiveDisplay(Node::Expr *fd, Node::Expr *arg) {
  nativeFunctionsUsed[NativeASMFunc::strlen_func] = true;
  visitExpr(arg);
  bool isSigned = static_cast<SymbolType *>(arg->asmType)->signedness ==
                  SymbolType::Signedness::SIGNED;
  switch (getByteSizeOfType(arg->asmType)) {
  case 1:
    push(Instr{.var = PopInstr{.where = "%al", .whereSize = DataSize::Byte},
               .type = InstrType::Pop},
         Section::Main);
    if (isSigned)
      pushLinker("movsbq %al, %rdi\n\t", Section::Main);
    else
      pushLinker("movzbq %al, %rdi\n\t", Section::Main);
    break;
  case 2:
    push(Instr{.var = PopInstr{.where = "%ax", .whereSize = DataSize::Word},
               .type = InstrType::Pop},
         Section::Main);
    if (isSigned)
      pushLinker("movswq %ax, %rdi\n\t", Section::Main);
    else
      pushLinker("movzwq %ax, %rdi\n\t", Section::Main);
    break;
  case 4:
    push(Instr{.var = PopInstr{.where = "%eax", .whereSize = DataSize::Dword},
               .type = InstrType::Pop},
         Section::Main);
    if (isSigned)
      pushLinker("movslq %eax, %rdi\n\t", Section::Main);
    else
      pushLinker("movzlq %eax, %rdi\n\t", Section::Main);
    break;
  case 8:
  default:
    push(Instr{.var = PopInstr{.where = "%rdi", .whereSize = DataSize::Qword},
               .type = InstrType::Pop},
         Section::Main);
    break;
  }
  // subtract rsp
  push(Instr{.var = SubInstr{.lhs = "%rsp",
                             .rhs = "$" + std::to_string(variableCount),
                             .size = DataSize::Qword},
             .type = InstrType::Sub},
       Section::Main);
  // Convert the number to a printable string
  if (isSigned) {
    push(
        Instr{.var = CallInstr{.name = "native_itoa"}, .type = InstrType::Call},
        Section::Main);
    nativeFunctionsUsed[NativeASMFunc::itoa] = true;
  } else {
    push(Instr{.var = CallInstr{.name = "native_uitoa"},
               .type = InstrType::Call},
         Section::Main);
    nativeFunctionsUsed[NativeASMFunc::uitoa] = true;
  }
  moveRegister("%rdi", "%rax", DataSize::Qword, DataSize::Qword);
  moveRegister("%rsi", "%rdi", DataSize::Qword, DataSize::Qword);
  // Now we have the integer string in %rax (assuming %rax holds the pointer to
  // the result)
  push(
      Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call},
      Section::Main);
  push(Instr{.var = AddInstr{.lhs = "%rsp",
                             .rhs = "$" + std::to_string(variableCount),
                             .size = DataSize::Qword},
             .type = InstrType::Sub},
       Section::Main);
  moveRegister("%rdx", "%rax", DataSize::Qword,
               DataSize::Qword); // Length of number string
  visitExpr(fd);
  popToRegister("%rdi");
  prepareSyscallWrite();
}

void codegen::handleFloatDisplay(Node::Expr *fd, Node::Expr *arg, int line,
                                 int pos) {
  // Still try printing something, to make it worth the trouble.
  handleLiteralDisplay(
      fd, new StringExpr(line, pos, "\"Printing floats is not supported yet.\"",
                         arg->file_id));
  std::string msg = "Printing floats is not supported yet.";
  handleError(line, pos, msg, "Codegen Error");
}

// Add 1
size_t codegen::getFileID(const std::string &file) {
  // check if fileIDs has the file using an iterator
  // Check if the flie path is absolute
  std::string absoluteFilePath = file;
  if (std::filesystem::path(file).is_relative()) {
    absoluteFilePath = std::filesystem::absolute(file).string();
  }
  auto it = std::find(fileIDs.begin(), fileIDs.end(),
                      absoluteFilePath); // Im assuming the standard library
                                         // function would be efficient here
  if (it != fileIDs.end()) {
    return std::distance(fileIDs.begin(), it);
  }
  fileIDs.push_back(absoluteFilePath);
  return fileIDs.size() - 1;
}

void codegen::pushDebug(size_t line, size_t file, long column) {
  // If not in debug mode, this funciton will pretty much be a massive nop.
  if (!debug)
    return;
  if (column != -1) {
    push(
        Instr{.var = LinkerDirective{.value = ".loc " + std::to_string(file) +
                                              " " + std::to_string(line) + " " +
                                              std::to_string(column) + "\n\t"},
              .type = InstrType::Linker},
        Section::Main);
  } else {
    push(Instr{.var = LinkerDirective{.value = ".loc " + std::to_string(file) +
                                               " " + std::to_string(line) +
                                               "\n\t"},
               .type = InstrType::Linker},
         Section::Main);
  }
}

// A real type, not the asmType
long int codegen::getByteSizeOfType(Node::Type *type) {
  switch (type->kind) {
  case ND_ARRAY_TYPE: {
    ArrayType *a = static_cast<ArrayType *>(type);
    if (a->constSize <= 0) {
      // Give up
      return 8;
    }
    return a->constSize * getByteSizeOfType(a->underlying);
  }
  case ND_POINTER_TYPE:
  case ND_FUNCTION_TYPE:
  case ND_FUNCTION_TYPE_PARAM:
    return 8;
  case ND_SYMBOL_TYPE: {
    SymbolType *sym = static_cast<SymbolType *>(type);
    if (structByteSizes.find(sym->name) != structByteSizes.end()) {
      return structByteSizes[sym->name].first;
    }
    if (typeSizes.find(sym->name) != typeSizes.end()) {
      return typeSizes[sym->name];
    }
    if (enumTable.find(sym->name) != enumTable.end()) {
      return 4;
    }
    if (sym->name.find("*") != std::string::npos)
      return 8;
    // Unknown type...
    // Hopefully this is unreachable!
    std::string msg =
        "Unknown type '" + sym->name +
        "'. For some reason, this type is not in the typeSizes map.";
    handleError(0, 0, msg, "Codegen Error");
    return 0; // Let the compiler know that this is an error
  }
  default:
    std::string msg = "Unknown type '" + std::to_string((int)type->kind) + "'.";
    handleError(0, 0, msg, "Codegen Error");
    return 0;
  }
};

std::string codegen::getUnderlying(Node::Type *type) {
  // Eventually, all underlying's will turn into SymbolType's, which is just the
  // name of the type.
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
    return type_to_diename(a->underlying) + "_arr" +
           std::to_string(a->constSize - 1);
  }
  if (type->kind == ND_SYMBOL_TYPE) { // Yes, structs are symbols too
    SymbolType *s = static_cast<SymbolType *>(type);
    std::string signedness = "";
    // check if builtin type
    bool builtin = typeSizes.find(s->name) != typeSizes.end();
    if (builtin) { // only builtin types can have signedness
      switch (s->signedness) {
      case SymbolType::Signedness::SIGNED:
        signedness = "_s";
        break;
      case SymbolType::Signedness::UNSIGNED:
        signedness = "_u";
        break;
      default:
      case SymbolType::Signedness::INFER:
        signedness = "_i";
        break;
      }
    }

    return s->name + signedness;
  }
  // Unreachable!
  return "";
};

// This will usually be a multiple of 8 because x64 rules!
// Multiple: 8
// 16 -> 16
// 17 -> 24
size_t codegen::round(size_t num, size_t multiple) {
  return (num + multiple - 1) / multiple * multiple;
};

// Get the number of bytes that `value` takes up when encoded in the LEB128
// format.
size_t codegen::sizeOfLEB(int64_t value) {
  size_t size = 0;
  bool more = true;

  while (more) {
    uint8_t byte = value & 0x7F; // Get the lowest 7 bits
    value >>= 7;                 // Arithmetic right shift for signed values

    // Determine if more bytes are needed
    if ((value == 0 && (byte & 0x40) == 0) ||
        (value == -1 && (byte & 0x40) != 0)) {
      more = false; // Terminate if no more bytes are needed
    }

    ++size; // Count the byte
  }

  return size;
}

DataSize codegen::intDataToSize(long int data) {
  switch (data) {
  case 1:
    return DataSize::Byte;
  case 2:
    return DataSize::Word;
  case 4:
    return DataSize::Dword;
  case 8:
  default:
    return DataSize::Qword;
  }
};

DataSize codegen::intDataToSizeFloat(long int data) {
  switch (data) {
  case 4:
  default:
    return DataSize::SS;
  case 8:
    return DataSize::SD;
  }
};
