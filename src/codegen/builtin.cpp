#include "gen.hpp"

// This file includes the codegen for the builtin functions, which is mostly just the mapping of a syscall
// ex: `@dis` -> `codegen::print` -> SYS_WRITE (automatically maps to the fd for stdout)
// ex: `@exit` -> `codegen::exit` -> SYS_EXIT
// `@import` -> `codegen::importDecl` -> `codegen::program` (the entry point of the new file)
void codegen::print(Node::Stmt *stmt) {
  PrintStmt *print = static_cast<PrintStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(print->line, stmt->file_id);

  for (Node::Expr *arg : print->args) {
    std::string argType = getUnderlying(arg->asmType);
    if (argType.find("*") == 0) {
      if (argType.find("char") == 1) { // Printing a char* will only print the first character. Printing full strings is reserved for the `str` type.
        visitExpr(arg);
        popToRegister("%rsi");
        // syscall id for write on x86 is 1
        moveRegister("%rax", "$1", DataSize::Qword, DataSize::Qword);
        // set rdi to 1 (file descriptor for stdout)
        moveRegister("%rdi", "$1", DataSize::Qword, DataSize::Qword);

        // Make syscall to write
        push(Instr{.var = Syscall({.name = "SYS_WRITE"}), .type = InstrType::Syscall}, Section::Main);
      } else {
        handleError(print->line, print->pos, "Cannot print pointer type. Dereference first or print as an address (int cast).", "Codegen Error");
      }
    } else
    if (argType == "str") {
      nativeFunctionsUsed[NativeASMFunc::strlen] = true;
      visitExpr(arg);
      popToRegister("%rsi"); // String address
      moveRegister("%rdi", "%rsi", DataSize::Qword, DataSize::Qword);
      push(Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call}, Section::Main);
      moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of string

      // syscall id for write on x86 is 1
      moveRegister("%rax", "$1", DataSize::Qword, DataSize::Qword);
      // set rdi to 1 (file descriptor for stdout)
      moveRegister("%rdi", "$1", DataSize::Qword, DataSize::Qword);

      // Make syscall to write
      push(Instr{.var = Syscall{.name = "SYS_WRITE"}, .type = InstrType::Syscall}, Section::Main);
    } else if (argType == "int" || argType == "char") { // Char's will be treated as the byte they are. They will be printed as their ASCII value. Ex: `@dis('A')` -> `65`
      nativeFunctionsUsed[NativeASMFunc::strlen] = true;
      nativeFunctionsUsed[NativeASMFunc::itoa] = true;
      visitExpr(arg);
      popToRegister("%rdi");
      push(Instr{.var = CallInstr{.name = "native_itoa"}, .type = InstrType::Call}, Section::Main); // Convert int to string
      moveRegister("%rdi", "%rax", DataSize::Qword, DataSize::Qword);
      moveRegister("%rsi", "%rdi", DataSize::Qword, DataSize::Qword);
      // Now we have the integer string in %rax (assuming %rax holds the pointer to the result)
      push(Instr{.var = CallInstr{.name = "native_strlen"}, .type = InstrType::Call}, Section::Main);
      moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of number string
      
      // syscall id for write on x86 is 1
      moveRegister("%rax", "$1", DataSize::Qword, DataSize::Qword);
      // set rdi to 1 (file descriptor for stdout)
      moveRegister("%rdi", "$1", DataSize::Qword, DataSize::Qword);

      // Make syscall to write
      push(Instr{.var = Syscall({.name = "SYS_WRITE"}), .type = InstrType::Syscall}, Section::Main);
    }
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
    std::cout << "Warning: File '" << s->name << "' already @link'd." << std::endl;
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
  Node::Type *toType = e->castee_type;
  std::string underlyingTo = getUnderlying(toType);
  Node::Type *fromType = e->castee->asmType;
  std::string underlyingFrom = getUnderlying(fromType);
  pushDebug(e->line, expr->file_id, e->pos);
  visitExpr(e->castee);
  if (toType->kind == ND_POINTER_TYPE && fromType->kind == ND_POINTER_TYPE) {
    // We are casting a pointer to a pointer
    // This is a no-op
    return;
  }
  if (toType->kind == ND_ARRAY_TYPE && fromType->kind == ND_ARRAY_TYPE) {
    // Array casted to an array? How does that work?
    if (getByteSizeOfType(toType) != getByteSizeOfType(fromType))
      handleError(e->line, e->pos, "Unable to cast arrays of different type sizes.", "Codegen Error", true);
    ArrayType *toArr = static_cast<ArrayType *>(toType);
    ArrayType *fromArr = static_cast<ArrayType *>(fromType);
    if (toArr->constSize != fromArr->constSize)
      handleError(e->line, e->pos, "Unable to cast arrays of different lengths.", "Codegen Error", true);
    // Well, since clearly the developer knew what they were doing, we will just give it to them!
    return;
  }
  // There are more pairs of types that can NOT be casted to one another
  if (fromType->kind == toType->kind && underlyingFrom == underlyingTo) return;
  if (fromType->kind == ND_SYMBOL_TYPE && toType->kind == ND_POINTER_TYPE && underlyingFrom == underlyingTo) {
    handleError(e->line, e->pos, "Cannot cast a symbol to a pointer. Please use the '&' operator instead.", "Codegen Error", true);
  }
  if (fromType->kind == ND_SYMBOL_TYPE && toType->kind == ND_POINTER_TYPE) {
    handleError(e->line, e->pos, "Cannot cast symbol to a pointer of different type. Please use the '&' operator and cast the result instead.", "Codegen Error", true);
  }
  if (fromType->kind == ND_POINTER_TYPE && toType->kind == ND_SYMBOL_TYPE && underlyingFrom == underlyingTo) {
    handleError(e->line, e->pos, "Cannot cast a pointer to a symbol. Please dereference instead.", "Codegen Error", true);
  }
  if (fromType->kind == ND_POINTER_TYPE && toType->kind == ND_SYMBOL_TYPE) {
    handleError(e->line, e->pos, "Cannot cast pointer to a symbol of different type. Please dereference and cast the result instead.", "Codegen Error", true);
  }
  if ((fromType->kind == ND_ARRAY_TYPE && toType->kind == ND_POINTER_TYPE)
   || (fromType->kind == ND_POINTER_TYPE && toType->kind == ND_ARRAY_TYPE)) {
    handleError(e->line, e->pos, "Unable to cast arrays and pointers.", "Codegen Error", true);
  }
  if ((toType->kind == ND_FUNCTION_TYPE)
   || (fromType->kind == ND_FUNCTION_TYPE)) {
    handleError(e->line, e->pos, "It is illegal to cast function types.", "Codegen Error", true);
  }
  // Great! At this point, we now know that the underlying types are symbol types (if not, they would be handled already)
  if (getUnderlying(fromType) == "unknown") {
    return; // Assume the dev knew what they were doing and push the original value
  }
  if (underlyingTo == "enum") {
    // Enums are truly just ints under-the-hood
    if (underlyingFrom != "int") {
      std::cerr << "Cannot cast non-int to enum" << std::endl;
      exit(-1);
    }
    return; // Do nothing (essentially just a void cast)
  }
  if (underlyingTo == "float") {
    // We are casting into a float.
    popToRegister("%rax"); // This actually doesnt matter but rax is fast and also our garbage disposal
    // Perform the conversion
    if (underlyingFrom == "int") {
      push(Instr{.var = ConvertInstr{.convType = ConvertType::SI2SS, .from = "%rax", .to = "%xmm1"},
                .type = InstrType::Convert},
          Section::Main);
      // push xmm0
      push(Instr{.var=PushInstr{.what="%xmm0",.whatSize=DataSize::SS},.type=InstrType::Push},Section::Main);
    }
    if (underlyingFrom == "float") {
      // Do nothing, it's already a float
      // Optimizer will know that this redundant push/pop is, of course, redundant
      pushRegister("%rax");
    }
  } else if (underlyingTo == "int") {
    if (underlyingFrom == "int" || underlyingFrom == "enum") {
      // Do nothing, it's already an int
      // Enums are explicitly ints under-the-hood as well
      return;
    }

    if (underlyingFrom == "float") {
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
  if (e->args.size() > intArgOrder.size()) {
    std::cerr << "Too many arguments in call - consider reducing them or moving them to a globally defined space." << std::endl;
    exit(-1);
  }
  int intArgCount = 0;
  int floatArgCount = 0;
  for (size_t i = 0; i < e->args.size(); i++) {
    // evaluate them
    codegen::visitExpr(e->args.at(i));
    // check if it is considered a float
    if (e->args[i]->asmType->kind == ND_SYMBOL_TYPE && getUnderlying(e->args[i]->asmType) == "float") {
      // it is a float
      popToRegister(floatArgOrder[floatArgCount++]); // ++ operator returns the original, so starting at 0 :smile:
    } else
      popToRegister(intArgOrder[intArgCount++]);
  }
  // In short, the PLT is a table of function pointers that are resolved at runtime.
  // This is used for dynamically linked executables, which is by default what we are doing.
  // https://stackoverflow.com/questions/5469274/what-does-plt-mean-here
  
  // NOTE: This is what linkers are for. They include these functions and resolve them. This means that @link's are always required for an @call.
  push(Instr{.var = CallInstr{.name = e->name + "@PLT"},
             .type = InstrType::Call},
       Section::Main);
  pushRegister("%rax");
}