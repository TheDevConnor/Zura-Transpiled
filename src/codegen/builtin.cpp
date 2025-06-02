#include <unordered_map>

#include "../typeChecker/type.hpp"
#include "gen.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"

void codegen::print(Node::Stmt *stmt) {
  OutputStmt *print = static_cast<OutputStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "print stmt"},
             .type = InstrType::Comment},
       Section::Main);
  pushDebug(print->line, stmt->file_id);

  for (Node::Expr *arg : print->args) {
    Node::Expr *optimizedArg = CompileOptimizer::optimizeExpr(arg);
    std::string argType = TypeChecker::type_to_string(optimizedArg->asmType);

    if (argType == "str" || argType == "*char") {
      return handleStrDisplay(print->fd, optimizedArg);
    } else if (optimizedArg->asmType->kind == ND_POINTER_TYPE) {
      return handlePtrDisplay(print->fd, arg, print->line, print->pos);
    } else if (optimizedArg->kind == ND_INT || optimizedArg->kind == ND_BOOL ||
               optimizedArg->kind == ND_CHAR ||
               optimizedArg->kind == ND_FLOAT ||
               optimizedArg->kind == ND_STRING) {
      return handleLiteralDisplay(print->fd, optimizedArg);
    } else if (TypeChecker::isIntBasedType(arg->asmType)) {
      return handlePrimitiveDisplay(print->fd, optimizedArg);
    } else if (argType == "float" || argType == "double") {
      return handleFloatDisplay(print->fd, optimizedArg, print->line,
                                print->pos);
    } else if (optimizedArg->asmType->kind == ND_ARRAY_TYPE) {
      return handleArrayDisplay(print->fd, optimizedArg, print->line,
                                print->pos);
    } else {
      handleError(print->line, print->pos,
                  "Cannot print type '" + argType + "'.", "Codegen Error");
    }
  }
}

void codegen::importDecl(Node::Stmt *stmt) {
  // Lex, parse, and generate code for the imported file
  // Keep track of its imports to ensure there are no circular dependencies.

  ImportStmt *s = static_cast<ImportStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "Import file '" + s->name + "'."},
             .type = InstrType::Comment},
       Section::Main);
  std::string preFilePath = node.current_file;
  node.current_file = s->name;
  codegen::program(s->stmt);
  node.current_file = preFilePath;
};

void codegen::linkFile(Node::Stmt *stmt) {
  LinkStmt *s = static_cast<LinkStmt *>(stmt);
  if (linkedFiles.find(s->name) != linkedFiles.end()) {
    // It's not the end of the world.
    std::cout << "Warning: Library '" << s->name << "' already @link'd."
              << std::endl;
  } else {
    linkedFiles.insert(s->name);
  }
}

void codegen::externName(Node::Stmt *stmt) {
  ExternStmt *s = static_cast<ExternStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "Extern name '" + s->name + "'."},
             .type = InstrType::Comment},
       Section::Main);
  // Push the extern directive to the front of the section
  if (externalNames.find(s->name) != externalNames.end()) {
    std::cout << "Error: Name '" << s->name << "' already @extern'd."
              << std::endl;
    return;
  }
  // i did, it works we don't touch now lmao
  if (s->externs.size() > 0) {
    for (std::string &ext : s->externs) {
      text_section.emplace(
          text_section.begin(),
          Instr{.var = LinkerDirective{.value = ".extern " + ext + "\n"},
                .type = InstrType::Linker});
      externalNames.insert(ext);
    }
  } else {
    text_section.emplace(
        text_section.begin(),
        Instr{.var = LinkerDirective{.value = ".extern " + s->name + "\n"},
              .type = InstrType::Linker});
    externalNames.insert(s->name);
  }
  if (debug)
    text_section.emplace(
        text_section.begin(),
        Instr{.var =
                  LinkerDirective{// NOTE: I'm not sure if this .loc will be
                                  // registered properly.
                                  .value = ".loc " +
                                           std::to_string(s->file_id) + " " +
                                           std::to_string(s->line) + " " +
                                           std::to_string(s->pos) + "\n\t"},
              .type = InstrType::Linker});
  text_section.emplace(
      text_section.begin(),
      Instr{.var = Comment{.comment = "Include external function name '" +
                                      s->name + "'."},
            .type = InstrType::Comment});
  externalNames.insert(s->name);
}

void codegen::cast(Node::Expr *expr) {
  CastExpr *e = static_cast<CastExpr *>(expr);
  Node::Type *to = e->castee_type;
  Node::Type *from = e->castee->asmType;

  if (to->kind == ND_POINTER_TYPE && from->kind == ND_POINTER_TYPE) {
    PointerType *toPtr = static_cast<PointerType *>(to);
    PointerType *fromPtr = static_cast<PointerType *>(from);
    if (toPtr->underlying->kind != ND_SYMBOL_TYPE ||
        fromPtr->underlying->kind != ND_SYMBOL_TYPE) {
      handleError(e->line, e->pos,
                  "Pointer casts on non-base types (e.g. int, float, bool...) "
                  "are not allowed.",
                  "Codegen Error");
      return;
    }
    if (getUnderlying(toPtr->underlying) == "unknown" ||
        getUnderlying(fromPtr->underlying) == "unknown" ||
        getUnderlying(toPtr->underlying) == "void" ||
        getUnderlying(fromPtr->underlying) == "void") {
      // Execute the code but do not type cast anything
      visitExpr(e->castee);
      return; // Assume the dev knew what they were doing and push the original
              // value
    }
    if (getUnderlying(toPtr->underlying) !=
        getUnderlying(fromPtr->underlying)) {
      handleError(e->line, e->pos,
                  "Cannot cast pointers from different types. (unimplemented)",
                  "Codegen Error");
      return;
    }
  }

  SymbolType *toType = static_cast<SymbolType *>(to);
  SymbolType *fromType = static_cast<SymbolType *>(from);

  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->castee);
  if (fromType->name == "unknown") {
    return; // Assume the dev knew what they were doing and push the original
            // value
  }
  if (toType->name == "enum") {
    // Enums are truly just ints under-the-hood
    if (fromType->name != "long") {
      std::cerr << "Cannot cast non-int to enum" << std::endl;
      exit(-1);
    }
    return; // Do nothing (essentially just a void cast)
  }
  if (toType->name == "float") {
    // We are casting into a float.
    popToRegister("%rax"); // This actually doesnt matter but rax is fast and
                           // also our garbage disposal
    // Perform the conversion
    if (TypeChecker::isIntBasedType(fromType)) {
      push(Instr{.var = ConvertInstr{.toSize = DataSize::SS,
                                     .convType = ConvertType::SI2SS,
                                     .from = "%rax",
                                     .to = "%xmm0"},
                 .type = InstrType::Convert},
           Section::Main);
      // push xmm0
      push(Instr{.var = PushInstr{.what = "%xmm0", .whatSize = DataSize::SS},
                 .type = InstrType::Push},
           Section::Main);
      return;
    }
    if (fromType->name == "float") {
      // Do nothing, it's already a float
      // Optimizer will know that this redundant push/pop is, of course,
      // redundant
      pushRegister("%rax");
      return;
    }
  } else if (TypeChecker::isIntBasedType(toType)) {
    if (TypeChecker::isIntBasedType(fromType)) {
      std::string fromReg = "%rax";
      DataSize fromSize = DataSize::Qword;
      switch (getByteSizeOfType(fromType)) {
      case 1:
        fromReg = "%al";
        fromSize = DataSize::Byte;
        break;
      case 2:
        fromReg = "%ax";
        fromSize = DataSize::Word;
        break;
      case 4:
        fromReg = "%eax";
        fromSize = DataSize::Dword;
        break;
      case 8:
      default:
        fromReg = "%rax";
        fromSize = DataSize::Qword;
        break;
      }
      push(Instr{.var = PopInstr{.where = fromReg, .whereSize = fromSize},
                 .type = InstrType::Pop},
           Section::Main);
      if (getByteSizeOfType(fromType) < getByteSizeOfType(toType)) {
        if (toType->signedness == SymbolType::Signedness::SIGNED) {
          // Do a sign extend
          switch (getByteSizeOfType(fromType)) {
          case 2:
            pushLinker("movsbl " + fromReg + ", %ax\n\t", Section::Main);
            break;
          case 4:
            pushLinker("movslq " + fromReg + ", %rax\n\t", Section::Main);
            break;
          default:
            break;
          }
          // Do a zero extend (basically do nothing actually)
          switch (getByteSizeOfType(fromType)) {
          case 1:
            pushLinker("movzbl " + fromReg + ", %eax\n\t", Section::Main);
            break;
          case 2:
            pushLinker("movzwl " + fromReg + ", %eax\n\t", Section::Main);
            break;
          case 4:
            pushLinker("movl " + fromReg + ", %eax\n\t", Section::Main);
            break;
          default:
            break;
          }
        }
        // No matter how its signed, we must still push in order to return
        pushRegister("%rax");
      }
      // Do nothing, it's already an int
      // Enums are explicitly ints under-the-hood as well
      return;
    }

    if (fromType->name == "float" || fromType->name == "double") {
      // Pop to the register
      push(Instr{.var = PopInstr{.where = "%xmm0",
                                 .whereSize = intDataToSizeFloat(
                                     getByteSizeOfType(fromType))},
                 .type = InstrType::Pop},
           Section::Main);
      // Now its in xmm0, we must CONVERT!!
      ConvertType type = ConvertType::SS2SI; // Most common
      switch (getByteSizeOfType(fromType)) {
      case 4:
      default:
        type = ConvertType::SS2SI;
        break;
      case 8:
        type = ConvertType::SD2SI;
        break;
      }
      push(Instr{.var = ConvertInstr{.toSize = intDataToSize(
                                         getByteSizeOfType(toType)),
                                     .convType = type,
                                     .from = "%xmm0",
                                     .to = "%rax"},
                 .type = InstrType::Convert},
           Section::Main);
      switch (getByteSizeOfType(toType)) {
      // the final push instr
      case 1:
        push(Instr{.var = PushInstr{.what = "%al", .whatSize = DataSize::Byte},
                   .type = InstrType::Push},
             Section::Main);
        break;
      case 2:
        push(Instr{.var = PushInstr{.what = "%ax", .whatSize = DataSize::Word},
                   .type = InstrType::Push},
             Section::Main);
        break;
      case 4:
        push(
            Instr{.var = PushInstr{.what = "%eax", .whatSize = DataSize::Dword},
                  .type = InstrType::Push},
            Section::Main);
        break;
      case 8:
      default:
        push(
            Instr{.var = PushInstr{.what = "%rax", .whatSize = DataSize::Qword},
                  .type = InstrType::Push},
            Section::Main);
        break;
      }
    }
    return;
  }
}

void codegen::externalCall(Node::Expr *expr) {
  // Basically like a normal function call
  // ... Minus the "usr_" prefix
  ExternalCall *e = static_cast<ExternalCall *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);
  // Push each argument one by one.
  int intArgCount = 0;
  int floatArgCount = 0;
  for (size_t i = 0; i < e->args.size(); i++) {
    // evaluate them
    visitExpr(e->args.at(i));
    if (getUnderlying(e->args[i]->asmType) == "float" &&
        e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
      popToRegister(floatArgOrder[floatArgCount++]);
    } else {
      popToRegister(intArgOrder[intArgCount++]);
    }
  }
  // Call the function
  push(Instr{.var =
                 CallInstr{
                     .name =
                         e->name +
                         "@PLT"}, // the @PLT is for dynamic linking where the
                                  // function is not in defined in this current
                                  // asm file and is imported elsewhere
             .type = InstrType::Call},
       Section::Main);
  pushRegister("%rax");
}

void codegen::allocExpr(Node::Expr *expr) {
  AllocMemoryExpr *alloc = static_cast<AllocMemoryExpr *>(expr);
  visitExpr(alloc->bytesToAlloc);
  /*
  movq $9, %rax #
  movq $0, %rdi #
  movq $1024, %rsi # this is the one we care about
  movq $3, %rdx #
  movq $34, %r10 #
  movq $-1, %r8
  movq $0, %r9 #
  */
  popToRegister("%rsi");
  moveRegister("%rax", "$9", DataSize::Qword, DataSize::Qword); // Syscall no
  moveRegister("%rdi", "$0", DataSize::Qword,
               DataSize::Qword); // We don't care where you put the memory, just
                                 // alloc anywhere
  moveRegister("%rdx", "$3", DataSize::Qword,
               DataSize::Qword); // Protection flags: PROT_READ | PROT_WRITE
                                 // (constant for now)
  moveRegister("%r10", "$34", DataSize::Qword,
               DataSize::Qword); // Memory flags: MAP_PRIVATE | MAP_ANONYMOUS
  moveRegister("%r8", "$-1", DataSize::Qword,
               DataSize::Qword); // There is no file descriptor associated here-
                                 // the memory is anonymous
  moveRegister("%r9", "$0", DataSize::Qword,
               DataSize::Qword); // Offset- Once again, the memory is anonymous,
                                 // so we do not care
  push(Instr{.var = Syscall{.name = "SYS_MMAP"}, .type = InstrType::Syscall},
       Section::Main);
  pushRegister("%rax");
};

void codegen::freeExpr(Node::Expr *expr) {
  FreeMemoryExpr *free = static_cast<FreeMemoryExpr *>(expr);

  // Very simple, this one!
  visitExpr(free->whatToFree);  // rdi
  visitExpr(free->bytesToFree); // rsi
  popToRegister("%rsi");
  popToRegister("%rdi");
  // rax is constant- the syscall number
  moveRegister("%rax", "$11", DataSize::Qword,
               DataSize::Qword); // Syscall number
  push(Instr{.var = Syscall{.name = "SYS_MUNMAP"}, .type = InstrType::Syscall},
       Section::Main);
  pushRegister("%rax");
};

void codegen::sizeofExpr(Node::Expr *expr) {
  SizeOfExpr *s = static_cast<SizeOfExpr *>(expr);

  // Define a lookup for type sizes
  size_t size = getByteSizeOfType(s->whatToSizeOf->asmType);

  // Push the size to the stack
  push(Instr{.var = PushInstr{.what = "$" + std::to_string(size),
                              .whatSize = DataSize::Qword},
             .type = InstrType::Push},
       Section::Main);

  // Push the size to the stack
  push(
      Instr{.var = Comment{.comment =
                               "Sizeof " +
                               static_cast<IdentExpr *>(s->whatToSizeOf)->name +
                               " is " + std::to_string(size) + " bytes."},
            .type = InstrType::Comment},
      Section::Main);
}

void codegen::memcpyExpr(Node::Expr *expr) {
  MemcpyExpr *m = static_cast<MemcpyExpr *>(expr);

  // Evaluate the arguments
  visitExpr(m->dest);
  visitExpr(m->src);
  visitExpr(m->bytes);

  // Pop the values into registers
  popToRegister("%rdx"); // bytes
  popToRegister("%rsi"); // src
  popToRegister("%rdi"); // dest

  // Emit the memcpy instruction
  push(Instr{.var = Comment{.comment = "memcpy(dest, src, bytes)"},
             .type = InstrType::Comment},
       Section::Main);
  push(
      Instr{.var = CallInstr{.name = "native_memcpy"}, .type = InstrType::Call},
      Section::Main);
  nativeFunctionsUsed[NativeASMFunc::memcpy_func] = true;

  // Push the return value to the stack
  pushRegister("%rax");
}

void codegen::getArgcExpr(Node::Expr *expr) {
  // Push debug
  GetArgcExpr *e = static_cast<GetArgcExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  // yes, that's it LOL
  pushRegister(".Largc(%rip)");
  useArguments = true;
}

void codegen::getArgvExpr(Node::Expr *expr) {
  // Push debug
  GetArgvExpr *e = static_cast<GetArgvExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  pushRegister(".Largv(%rip)");
  useArguments = true;
}

void codegen::strcmp(Node::Expr *expr) {
  StrCmp *s = static_cast<StrCmp *>(expr);
  pushDebug(s->line, expr->file_id, s->pos);

  // Push the first string
  visitExpr(s->v1);
  popToRegister("%rdi");

  // Push the second string
  visitExpr(s->v2);
  popToRegister("%rsi");

  // call the native strcmp function
  // Subtract variable count before call because of course we do
  long long offsetAmount = round(variableCount - 8, 8);
  if (offsetAmount)
    push(Instr{.var = SubInstr{.lhs = "%rsp",
                               .rhs = "$" + std::to_string(offsetAmount),
                               .size = DataSize::Qword},
               .type = InstrType::Sub},
         Section::Main);
  push(
      Instr{.var = CallInstr{.name = "native_strcmp"}, .type = InstrType::Call},
      Section::Main);
  nativeFunctionsUsed[NativeASMFunc::strcmp] = true;
  if (offsetAmount)
    push(Instr{.var = AddInstr{.lhs = "%rsp",
                               .rhs = "$" + std::to_string(offsetAmount),
                               .size = DataSize::Qword},
               .type = InstrType::Sub},
         Section::Main);
  // The return value is in %rax, so we can just push it
  push(Instr{.var =
                 PushInstr{
                     .what = "%al",
                     .whatSize = DataSize::Byte // booleans
                 },
             .type = InstrType::Push},
       Section::Main);
}

void codegen::openExpr(Node::Expr *expr) {
  OpenExpr *e = static_cast<OpenExpr *>(expr);
  pushDebug(e->line, expr->file_id, e->pos);

  visitExpr(e->filename);
  // DEAL WITH THESE LATER (VERY IMPORANT IG)
  // visitExpr(e->canRead);
  // visitExpr(e->canWrite);
  // visitExpr(e->canCreate);

  // Default flags: O_RDWR | O_CREAT | O_TRUNC
  // values:          2    |   100   |  1000   = 578

  // Create the syscall
  popToRegister("%rdi");
  if (e->canRead == nullptr && e->canWrite == nullptr &&
      e->canCreate == nullptr) {
    // The user did not specify any arguments. By default, they are Can Read,
    // Can Write, and Can Create
    moveRegister("%rsi", "$578", DataSize::Qword, DataSize::Qword);
  } else {
    bool canReadLiteral = false;
    bool canWriteLiteral = false;
    bool canCreateLiteral = false;
    if (e->canRead == nullptr)
      canReadLiteral = true; // Effectively, you wrote 'true'
    if (e->canWrite == nullptr)
      canWriteLiteral = true;
    if (e->canCreate == nullptr)
      canCreateLiteral = true;

    if (e->canRead != nullptr)
      if (e->canRead->kind == ND_BOOL)
        canReadLiteral = true;
    if (e->canWrite != nullptr)
      if (e->canWrite->kind == ND_BOOL)
        canWriteLiteral = true;
    if (e->canCreate != nullptr)
      if (e->canCreate->kind == ND_BOOL)
        canCreateLiteral = true;
    if (canReadLiteral && canWriteLiteral && canCreateLiteral) {
      // We can run these values and evaluate them in comptime
      bool canReadValue;
      bool canWriteValue;
      bool canCreateValue;
      if (e->canRead == nullptr)
        canReadValue = true;
      else
        canReadValue = static_cast<BoolExpr *>(e->canRead)->value;

      if (e->canWrite == nullptr)
        canWriteValue = true;
      else
        canWriteValue = static_cast<BoolExpr *>(e->canWrite)->value;

      if (e->canCreate == nullptr)
        canCreateValue = true;
      else
        canCreateValue = static_cast<BoolExpr *>(e->canCreate)->value;
      // read | write | create
      //  02 |  0100  | 01000
      const static int canRead = 02;
      const static int canWrite = 0100;
      const static int canCreate = 01000;
      moveRegister("%rsi",
                   "$" + std::to_string((canReadValue ? canRead : 0) |
                                        (canWriteValue ? canWrite : 0) |
                                        (canCreateValue ? canCreate : 0)),
                   DataSize::Qword, DataSize::Qword);
    } else {
      // Some values are literals, some are not
      // I could personally not care less about what you specify, so how about
      // we return the default
      moveRegister("%rsi", "$578", DataSize::Qword, DataSize::Qword);
    }
  }
  // mode_t mode = S_IRUSR | S_IWUSR | S_IROTH
  // values: total = 388

  // Have you ever wanted to run a shell file but had to run `chmod +x file.sh`
  // first? This rdx register, 'mode', says who has the permissions (like 'x'
  // for 'execute') on the file I'm not sure why this is needed, because the
  // file literally wouldn't be opened if you didn't have permission, but I have
  // to supply this otherwise the syscall will fail.
  moveRegister("%rdx", "$388", DataSize::Qword, DataSize::Qword);
  moveRegister("%rax", "$2", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_OPEN"}, .type = InstrType::Syscall},
       Section::Main);
  pushRegister("%rax");
};

void codegen::inputStmt(Node::Stmt *stmt) {
  InputStmt *s = static_cast<InputStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "input statement"},
             .type = InstrType::Comment},
       Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Now that we printed the "prompt", we now have to make the syscall using the
  // values do in reverse order because one of these may screw up the values of
  // the registers
  visitExpr(s->bufferOut); // rsi (should be leaq'd)
  visitExpr(s->maxBytes);  // rdx
  visitExpr(s->fd);        // rdi
  popToRegister("%rdi");   // rdi
  popToRegister("%rdx");   // rdx
  popToRegister("%rsi");   // rsi
  // RAX and RDI are constant in this case- constant syscall number and constant
  // file descriptor (fd of stdin is 0)
  push(Instr{.var = XorInstr{.lhs = "%rax", .rhs = "%rax"},
             .type = InstrType::Xor},
       Section::Main);
  push(Instr{.var = Syscall{.name = "SYS_READ"}, .type = InstrType::Syscall},
       Section::Main);
}

void codegen::closeStmt(Node::Stmt *stmt) {
  CloseStmt *s = static_cast<CloseStmt *>(stmt);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Now that we printed the "prompt", we now have to make the syscall using the
  // values do in reverse order because one of these may screw up the values of
  // the registers
  visitExpr(s->fd);      // rdi
  popToRegister("%rdi"); // rdi
  // RAX is constant in this case- constant syscall number
  // Stupid AI! Rax is not 0...
  moveRegister("%rax", "$3", DataSize::Qword, DataSize::Qword);
  push(Instr{.var = Syscall{.name = "SYS_CLOSE"}, .type = InstrType::Syscall},
       Section::Main);
}
