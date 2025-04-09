#include "gen.hpp"
#include "optimizer/instr.hpp"
#include "optimizer/compiler.hpp"
#include "../typeChecker/type.hpp"
#include <unordered_map>

void codegen::print(Node::Stmt *stmt) {
  OutputStmt *print = static_cast<OutputStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(print->line, stmt->file_id);

  for (Node::Expr *arg : print->args) {
    std::string argType = TypeChecker::type_to_string(arg->asmType);
    Node::Expr *optimizedArg = CompileOptimizer::optimizeExpr(arg);
    if (optimizedArg->asmType->kind == ND_POINTER_TYPE)
      handlePtrDisplay(print->fd, arg, print->line, print->pos);
    else if (optimizedArg->kind == ND_INT || optimizedArg->kind == ND_BOOL || optimizedArg->kind == ND_CHAR || optimizedArg->kind == ND_FLOAT || optimizedArg->kind == ND_STRING)
      handleLiteralDisplay(print->fd, optimizedArg);
    else if (argType == "str")
      handleStrDisplay(print->fd, optimizedArg);
    else if (TypeChecker::isIntBasedType(arg->asmType))
      handlePrimitiveDisplay(print->fd, optimizedArg);
    else if (argType == "float" || argType == "double")
      handleFloatDisplay(print->fd, optimizedArg, print->line, print->pos);
    else if (optimizedArg->asmType->kind == ND_ARRAY_TYPE) // must always be a char array, no other type of arr is allowed (char[] are also literally just char* anyway so lol)
      handleArrayDisplay(print->fd, optimizedArg, print->line, print->pos);
    else
      handleError(print->line, print->pos,
                  "Cannot print type '" + argType + "'.", "Codegen Error");
  }
}

void codegen::importDecl(Node::Stmt *stmt) {
  // Lex, parse, and generate code for the imported file
  // Keep track of its imports to ensure there are no circular dependencies.
  
  ImportStmt *s = static_cast<ImportStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "Import file '" + s->name + "'."}, .type = InstrType::Comment}, Section::Main);
  codegen::program(s->stmt);
};

void codegen::linkFile(Node::Stmt *stmt) {
  LinkStmt *s = static_cast<LinkStmt *>(stmt);
  if (linkedFiles.find(s->name) != linkedFiles.end()) {
    // It's not the end of the world.
    std::cout << "Warning: Library '" << s->name << "' already @link'd." << std::endl;
  } else {
    linkedFiles.insert(s->name);
  }
}

void codegen::externName(Node::Stmt *stmt) {
  ExternStmt *s = static_cast<ExternStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "Extern name '" + s->name + "'."}, .type = InstrType::Comment}, Section::Main);
  // Push the extern directive to the front of the section
  if (externalNames.find(s->name) != externalNames.end()) { 
    std::cout << "Error: Name '" << s->name << "' already @extern'd." << std::endl;
    return;
  } 
  // i did, it works we don't touch now lmao
  if (s->externs.size() > 0) {
    for (std::string &ext : s->externs) {
      text_section.emplace(text_section.begin(), Instr{.var = LinkerDirective{.value = ".extern " + ext + "\n"}, .type = InstrType::Linker});
      externalNames.insert(ext);
    }
  } else {
    text_section.emplace(text_section.begin(), Instr{.var = LinkerDirective{.value = ".extern " + s->name + "\n"}, .type = InstrType::Linker});
    externalNames.insert(s->name);
  }
  if (debug) text_section.emplace(text_section.begin(), Instr{.var = LinkerDirective{ // NOTE: I'm not sure if this .loc will be registered properly.
    .value = 
      ".loc " + std::to_string(s->file_id) + 
      " " + std::to_string(s->line) + 
      " " + std::to_string(s->pos) + 
      "\n\t"},
    .type = InstrType::Linker});
  text_section.emplace(text_section.begin(), Instr{.var = Comment{.comment = "Include external function name '" + s->name + "'."}, .type = InstrType::Comment});
  externalNames.insert(s->name);
}

void codegen::cast(Node::Expr *expr) {
  CastExpr *e = static_cast<CastExpr *>(expr);
  Node::Type *to = e->castee_type;
  Node::Type *from = e->castee->asmType;

  if (to->kind == ND_POINTER_TYPE && from->kind == ND_POINTER_TYPE) {
    PointerType *toPtr = static_cast<PointerType *>(to);
    PointerType *fromPtr = static_cast<PointerType *>(from);
    if (toPtr->underlying->kind != ND_SYMBOL_TYPE || fromPtr->underlying->kind != ND_SYMBOL_TYPE) {
      handleError(e->line, e->pos, "Pointer casts on non-base types (e.g. int, float, bool...) are not allowed.", "Codegen Error");
      return;
    }
    if (getUnderlying(toPtr->underlying) == "unknown" || getUnderlying(fromPtr->underlying) == "unknown"
     || getUnderlying(toPtr->underlying) == "void" || getUnderlying(fromPtr->underlying) == "void") {
      // Execute the code but do not type cast anything
      visitExpr(e->castee);
      return; // Assume the dev knew what they were doing and push the original value
    }
    if (getUnderlying(toPtr->underlying) != getUnderlying(fromPtr->underlying)) {
      handleError(e->line, e->pos, "Cannot cast pointers from different types. (unimplemented)", "Codegen Error");
      return;
    }
  }

  SymbolType *toType = static_cast<SymbolType *>(to);
  SymbolType *fromType = static_cast<SymbolType *>(from);

  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->castee);
  if (fromType->name == "unknown") {
    return; // Assume the dev knew what they were doing and push the original value
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
    popToRegister("%rax"); // This actually doesnt matter but rax is fast and also our garbage disposal
    // Perform the conversion
    if (fromType->name == "int") {
      push(Instr{.var = ConvertInstr{.convType = ConvertType::SI2SS, .from = "%rax", .to = "%xmm1"},
                .type = InstrType::Convert},
          Section::Main);
      // push xmm0
      push(Instr{.var=PushInstr{.what="%xmm0",.whatSize=DataSize::SS},.type=InstrType::Push},Section::Main);
    }
    if (fromType->name == "float") {
      // Do nothing, it's already a float
      // Optimizer will know that this redundant push/pop is, of course, redundant
      pushRegister("%rax");
    }
  } else if (toType->name == "int") {
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
      push(Instr{.var = PopInstr{.where = fromReg, .whereSize = fromSize}, .type = InstrType::Pop}, Section::Main);
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

    if (fromType->name == "float") {
      popToRegister("%xmm0");
      // Perform the conversion (always truncate for now)
      push(Instr{.var = ConvertInstr{.convType = ConvertType::TSS2SI, .from = "%xmm0", .to = "%rax"},
                .type = InstrType::Convert},
          Section::Main);
      // push rax - its ok becasue rax holds an int now! it fits on our int-based stack! woohoo!!!!
      pushRegister("%rax");
    }
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
    if (getUnderlying(e->args[i]->asmType) == "float" && e->args[i]->asmType->kind == ND_SYMBOL_TYPE) {
      popToRegister(floatArgOrder[floatArgCount++]);
    } else {
      popToRegister(intArgOrder[intArgCount++]);
    }
  }
  // Call the function
  push(Instr{.var = CallInstr{.name = e->name + "@PLT"}, // the @PLT is for dynamic linking where the function is
                                                         // not in defined in this current asm file and is imported elsewhere
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
  moveRegister("%rdi", "$0", DataSize::Qword, DataSize::Qword); // We don't care where you put the memory, just alloc anywhere
  moveRegister("%rdx", "$3", DataSize::Qword, DataSize::Qword); // Protection flags: PROT_READ | PROT_WRITE (constant for now)
  moveRegister("%r10", "$34", DataSize::Qword, DataSize::Qword); // Memory flags: MAP_PRIVATE | MAP_ANONYMOUS
  moveRegister("%r8", "$-1", DataSize::Qword, DataSize::Qword); // There is no file descriptor associated here- the memory is anonymous
  moveRegister("%r9", "$0", DataSize::Qword, DataSize::Qword); // Offset- Once again, the memory is anonymous, so we do not care
  push(Instr{.var=Syscall{.name="SYS_MMAP"},.type=InstrType::Syscall},Section::Main);
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
  moveRegister("%rax", "$11", DataSize::Qword, DataSize::Qword); // Syscall number
  push(Instr{.var=Syscall{.name="SYS_MUNMAP"},.type=InstrType::Syscall},Section::Main);
  pushRegister("%rax");
};

void codegen::sizeofExpr(Node::Expr *expr) {
  SizeOfExpr *s = static_cast<SizeOfExpr *>(expr);

  // Define a lookup for type sizes
  size_t size = getByteSizeOfType(s->whatToSizeOf->asmType);

  // Push the size to the stack
  push(Instr{.var=PushInstr{.what="$"+std::to_string(size),.whatSize=DataSize::Qword},.type=InstrType::Push},Section::Main);

  // Push the size to the stack
  push(Instr{.var=Comment{.comment="Sizeof " + static_cast<IdentExpr *>(s->whatToSizeOf)->name + " is " + std::to_string(size) + " bytes."},.type=InstrType::Comment},Section::Main);
}

void codegen::memcpyExpr(Node::Expr *expr) {
  MemcpyExpr *m = static_cast<MemcpyExpr *>(expr);

  // Evaluate the arguments
  visitExpr(m->dest);
  visitExpr(m->src);
  visitExpr(m->bytes);

  // Pop the values into registers
  popToRegister("%rdx");  // bytes
  popToRegister("%rsi");  // src
  popToRegister("%rdi");  // dest

  // Emit the memcpy instruction
  push(Instr{.var = Comment{.comment = "memcpy(dest, src, bytes)"}, .type = InstrType::Comment}, Section::Main);
  push(Instr{.var = CallInstr{.name = "native_memcpy"}, .type = InstrType::Call}, Section::Main);
  nativeFunctionsUsed[NativeASMFunc::memcpy] = true;

  // Push the return value to the stack
  pushRegister("%rax");
}