#include "../helper/error/error.hpp"  
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"
#include "../common.hpp"
#include "gen.hpp"

#include <iostream>
#include <sys/cdefs.h>

void codegen::visitStmt(Node::Stmt *stmt) {
  // compiler optimize the statement
  Node::Stmt *realStmt = CompileOptimizer::optimizeStmt(stmt);
  StmtHandler handler = lookup(stmtHandlers, realStmt->kind);
  if (handler) {
    handler(realStmt);
  }
}

void codegen::expr(Node::Stmt *stmt) {
  ExprStmt *s = static_cast<ExprStmt *>(stmt);
  // Just evaluate the expression
  codegen::visitExpr(s->expr);
  text_section.pop_back();
};

void codegen::program(Node::Stmt *stmt) {
  ProgramStmt *s = static_cast<ProgramStmt *>(stmt);
  for (Node::Stmt *stm : s->stmt) {
    // check for a global variable declaration
    if (stm->kind == ND_VAR_STMT) {
      VarStmt *var = static_cast<VarStmt *>(stm);
      if (var->expr) {
        codegen::visitStmt(var);
      }
    }
    codegen::visitStmt(stm);
  }
};

void codegen::constDecl(Node::Stmt *stmt) {
  ConstStmt *s = static_cast<ConstStmt *>(stmt);
  // TODO: Make ConstDecl's function like constant (immutable) variables
  pushDebug(s->line, stmt->file_id, s->pos);
  codegen::visitStmt(s->value);
};

void codegen::funcDecl(Node::Stmt *stmt) {
  FnStmt *s = static_cast<FnStmt *>(stmt);
  int preStackSize = stackSize;

  isEntryPoint = (s->name == "main") ? true : false;
  std::string funcName = (isEntryPoint) ? "main" : "usr_" + s->name;


  // WOO YEAH BABY DEBUG TIME

  std::string dieLabel = ".Ldie" + std::to_string(dieCount++);
  if (debug) {
    SymbolType *st = static_cast<SymbolType *>(s->returnType);
    std::string asmName = st->name;
    if (s->params.size() > 0) {
      dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionParam); // Formal parameter
      if (st->name == "void") {
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionWithParamsVoid);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionWithParamsVoid) +
                  "\n.byte 1" // External - This means "is it a public function?"
                  "\n.long " + dieLabel + "_string\n"
                  ".byte " + std::to_string(s->file_id) + // File ID
                  "\n.byte " + std::to_string(s->line) + "\n" // Line number
                  ".byte " + std::to_string(s->pos) + "\n" // Line column
                  ".quad " + dieLabel + "_debug_start\n" // Low pc
                  ".quad " + dieLabel + "_debug_end - " + dieLabel + "_debug_start\n" // high pc
                  ".uleb128 0x01\n" // 1 byte is gonna follow
                  ".byte 0x9c\n"
                  , Section::DIE);
      } else {
        dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionWithParams);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionWithParams) +
                  "\n.byte 1" // External - This means "is it a public function?"
                  "\n.long " + dieLabel + "_string\n"
                  ".byte " + std::to_string(s->file_id) + // File ID
                  "\n.byte " + std::to_string(s->line) + "\n" // Line number
                  ".byte " + std::to_string(s->pos) + "\n" // Line column
                  ".long .L" + asmName + "_debug_type\n" // Return type (DW_AT_type)
                  ".quad " + dieLabel + "_debug_start\n" // Low pc
                  ".quad " + dieLabel + "_debug_end - " + dieLabel + "_debug_start\n" // high pc
                  ".uleb128 0x01\n" // 1 byte is gonna follow
                  ".byte 0x9c\n"
                  , Section::DIE);
      }
    } else {
      if (st->name == "void") {
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionNoParamsVoid);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionNoParamsVoid) +
                  "\n.byte 1"
                  "\n.long " + dieLabel + "_string\n"
                  ".byte " + std::to_string(s->file_id) + // File ID
                  "\n.byte " + std::to_string(s->line) + "\n" // Line number
                  ".byte " + std::to_string(s->pos) + "\n" // Line column
                  ".quad " + dieLabel + "_debug_start\n" // Low pc
                  ".quad " + dieLabel + "_debug_end - " + dieLabel + "_debug_start\n" // high pc
                  ".uleb128 0x01\n" // 1 byte is gonna follow
                  ".byte 0x9c\n"
                  , Section::DIE);
      } else {
        dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionNoParams);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionNoParams) +
                  "\n.byte 1"
                  "\n.long " + dieLabel + "_string\n"
                  ".byte " + std::to_string(s->file_id) + // File ID
                  "\n.byte " + std::to_string(s->line) + "\n" // Line number
                  ".byte " + std::to_string(s->pos) + "\n" // Line column
                  ".long .L" + asmName + "_debug_type\n" // Return type (DW_AT_type)
                  ".quad " + dieLabel + "_debug_start\n" // Low pc
                  ".quad " + dieLabel + "_debug_end - " + dieLabel + "_debug_start\n" // high pc
                  ".uleb128 0x01\n" // 1 byte is gonna follow
                  ".byte 0x9c\n"
                  , Section::DIE);
      }
    }
    pushLinker(dieLabel + "_string: .string \"" + funcName + "\"\n", Section::DIEString);
    push(Instr {.var = Label {.name = dieLabel + "_debug_start" }, .type = InstrType::Label }, Section::Main);
  }
  
  pushLinker("\n.type " + funcName + ", @function", Section::Main);
  pushLinker("\n.globl " + funcName + "\n", Section::Main); // All functions are global functions for now.
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},Section::Main);
  // push linker directive for the debug info (the line number)
  pushDebug(s->line, stmt->file_id, s->pos);
  push(Instr{.var=LinkerDirective{.value=".cfi_startproc\n\t"},.type=InstrType::Linker},Section::Main);

  stackSize = 0;
  // Define literally (do not adjust cfa for this)
  push(Instr{.var = PushInstr{.what = "%rbp", .whatSize = DataSize::Qword},.type = InstrType::Push},Section::Main);
  stackSize++;
  push(Instr{.var=LinkerDirective{.value=".cfi_def_cfa_offset 16\n\t"},.type=InstrType::Linker},Section::Main);
  push(Instr{.var = MovInstr{.dest = "%rbp",
    .src = "%rsp",
    .destSize = DataSize::Qword,
    .srcSize = DataSize::Qword},
    .type = InstrType::Mov},
  Section::Main);
  push(Instr{.var=LinkerDirective{.value=".cfi_def_cfa_register 6\n\t"},.type=InstrType::Linker},Section::Main);

  // Function args
  for (size_t i = 0; i < s->params.size(); i++) {
    // TODO: REPLACE THIS IMMEDIATELY!
    variableTable.insert({s->params.at(i).first, argOrder.at(i)});

    if (debug) {
      SymbolType *st = static_cast<SymbolType *>(s->params.at(i).second);
      pushLinker("\n.uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionParam) +
                 "\n.byte " + std::to_string(s->file_id) +
                 "\n.byte " + std::to_string(s->line) +
                 "\n.byte " + std::to_string(s->pos) +
                 "\n.long .L" + st->name + "_debug_type\n" +
                 "\n.uleb128 0x1" // 1 byte is gonna follow
                 "\n.byte " + std::to_string(dwarf::argOP_regs.at(argOrder.at(i)) + 80) + "\n"
                 "\n"
      , Section::DIE);
    }
  }

  // Do not push the lexical block
  dwarf::nextBlockDIE = false;
  codegen::visitStmt(s->block);
  dwarf::nextBlockDIE = true; // reset it
  
  // Check if last instruction was a "RET"
  if (text_section.back().type != InstrType::Ret) {
    // Push a ret anyway
    // Otherwise we SEGFAULTT
    push(Instr{.var = Ret{.fromWhere=funcName},.type = InstrType::Ret},Section::Main);
  }

  // Function ends with ret so we can't really push any other instructions.
  if (debug) {
    pushLinker(dieLabel + "_debug_end:\n", Section::Main);
    pushLinker(".byte 0\n", Section::DIE); // End of children
  }

  push(Instr{.var = LinkerDirective{.value = ".cfi_endproc\n"},.type = InstrType::Linker},Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" + funcName + "\n\t"},
              .type = InstrType::Linker},Section::Main);
};

void codegen::varDecl(Node::Stmt *stmt) {
  VarStmt *s = static_cast<VarStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "Variable declaration for '" + s->name + "'"},
              .type = InstrType::Comment},Section::Main);

  // push another .loc
  pushDebug(s->line, stmt->file_id, s->pos);


  int whereBytes = -variableCount;
  std::string where = std::to_string(whereBytes) + "(%rbp)";
  if (s->expr != nullptr) {
    // The first clause: If it's a literal, straight up, struct declaration.
    // It might be a pointer to struct declaration, but the underlyingType func makes it 
    // seem like a normal one instead.
    if (s->type->kind == ND_SYMBOL_TYPE && structByteSizes.find(getUnderlying(s->type)) != structByteSizes.end()) {
      // It's of type struct!
      // Basically ignore the part where we allocate memory for this thing.
      push(Instr{.var = SubInstr{.lhs = "%rsp", .rhs = "$" + std::to_string(structByteSizes[getUnderlying(s->type)].first)}, .type = InstrType::Sub}, Section::Main);
      declareStructVariable(s->expr, getUnderlying(s->type), variableCount);
      variableCount += structByteSizes[getUnderlying(s->type)].first;
      variableTable.insert({s->name, where});
    } else if (s->type->kind == ND_ARRAY_TYPE || s->type->kind == ND_ARRAY_AUTO_FILL) {
      declareArrayVariable(s->expr, static_cast<ArrayType *>(s->type)->constSize, s->name); // s->name so it can be inserted to variableTable, s->type so we know the byte sizes.
    } else {
      int whereBytes = -variableCount;
      visitExpr(s->expr);
      popToRegister(where); // For values small enough to fit in a register.
      variableCount += getByteSizeOfType(s->type);
      variableTable.insert({s->name, where});
    }
  } else {
    // Subtract from the stack pointer
    // NOTE: this isn't really where they are stored anyways so this line was kinda useleess!!!
    // push(Instr{.var = SubInstr{.lhs = "%rsp", .rhs = "$" + std::to_string(-whereBytes)}, .type = InstrType::Sub}, Section::Main);
  }
  // Update the symbol table with the variable's position

  push(Instr{.var = Comment{.comment = "End of variable declaration for '" + s->name + "'"},
              .type = InstrType::Comment},Section::Main);

  // Push DWARF DIE for variable declaration!!!!!
  if (!debug) return;
  std::string dieLabel = ".Ldie" + std::to_string(dieCount);
  // Get the type of the variable
  // (struct, array, pointer, or basic)
  std::string asmName = type_to_diename(s->type); // This handles things like _ptr's and _arr's, too!
  dwarf::useType(s->type);
  dwarf::useAbbrev(dwarf::DIEAbbrev::Variable);
  dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
  pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
            "\n.long " + dieLabel + "_string\n"
            ".byte " + std::to_string(s->file_id) + "\n" // File index
            ".byte " + std::to_string(s->line) + "\n" // Line number
            ".byte " + std::to_string(s->pos) + "\n" // Line column
            ".long .L" + asmName + "_debug_type\n" // Type - point to the DIE of the DW_TAG_base_type
            ".uleb128 0x02\n" // Length of data in location definition - 2 bytes long
            ".byte 0x91\n" // DW_OP_fbreg (first byte)

            ".sleb128 " + std::to_string(whereBytes - 16) + "\n"
  , Section::DIE);

  // DIE String pointer
  push(Instr{.var = Label{.name = dieLabel + "_string"}, .type = InstrType::Label}, Section::DIEString);
  pushLinker(".string \"" + s->name + "\"\n", Section::DIEString);
  dieCount++;
}

void codegen::block(Node::Stmt *stmt) {
  BlockStmt *s = static_cast<BlockStmt *>(stmt);
  // TODO: Track the number of variables and pop them off later
  // This should be handled by the IR when i get around to it though
  size_t preSS = stackSize;
  int64_t preVS = variableCount;
  scopes.push_back(std::pair(preSS, preVS));
  // AHHHH DWARF STUFF ::SOB::::::::::
  // push a .loc
  pushDebug(s->line, stmt->file_id, s->pos);
  // gotta love the ++i operator bro :D
  // if this was i++, we'd be fucking dead!!!!!!! YIPEEEE
  size_t thisDieCount = dieCount++;
  if (dwarf::nextBlockDIE) {
    if (debug) push(Instr{.var = Label{.name = ".Ldie" + std::to_string(thisDieCount) + "_begin"}, .type = InstrType::Label}, Section::Main);
    dwarf::useAbbrev(dwarf::DIEAbbrev::LexicalBlock);
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::LexicalBlock) +
              "\n.byte " + std::to_string(s->file_id) + // File ID
              "\n.byte " + std::to_string(s->line) + // Line number
              "\n.byte " + std::to_string(s->pos) + // Column number
              "\n.long .Ldie" + std::to_string(thisDieCount) + "_begin" // Low pc
              "\n.quad .Ldie" + std::to_string(thisDieCount) + "_end - .Ldie" + std::to_string(thisDieCount) + "_begin\n" // High pc
    , Section::DIE);
  }
  for (Node::Stmt *stm : s->stmts) {
    codegen::visitStmt(stm);
  }
  if (dwarf::nextBlockDIE) {
    pushLinker(".byte 0\n", Section::DIE); // End of children nodes of the scope
    if (debug) push(Instr{.var = Label{.name = ".Ldie" + std::to_string(thisDieCount) + "_end"}, .type = InstrType::Label}, Section::Main);
  }
  stackSize = scopes.at(scopes.size() - 1).first;
  variableCount = scopes.at(scopes.size() - 1).second;
  scopes.pop_back();
};

void codegen::ifStmt(Node::Stmt *stmt) {
  IfStmt *s = static_cast<IfStmt *>(stmt);
  std::string preconCount = std::to_string(conditionalCount++);

  push(Instr{.var = Comment{.comment = "if statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  BinaryExpr *cond = nullptr;
  if (s->condition->kind == ND_GROUP) {
    GroupExpr *group = static_cast<GroupExpr *>(s->condition);
    cond = (group->expr->kind == ND_BINARY) ? static_cast<BinaryExpr *>(group->expr) : nullptr;
  } else {
    cond = static_cast<BinaryExpr *>(s->condition);
  }

  if (!cond) {
    // Handle non-binary conditions
    visitExpr(s->condition);
    popToRegister("%rcx");
    pushLinker("testq %rcx, %rcx\n\t", Section::Main);
    // Jump-if-zero only works when the zero flag is set.
    // ZF is only set when using "test" rather than "cmp", which affects
    // totally different flags for some reason.
    push(Instr{.var = JumpInstr{.op = JumpCondition::NotZero, .label = ".Lconditional" + preconCount}, .type = InstrType::Jmp}, Section::Main);
  } else {
    // Process binary expression
    processBinaryExpression(cond, preconCount, ".Lconditional");
  }

  // Jump to "main" label if condition is false (fall through)
  std::string elseLabel = ".Lelse" + preconCount;
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = elseLabel}, .type = InstrType::Jmp}, Section::Main);

  // True condition block: label and code for 'thenStmt'
  push(Instr{.var = Label{.name = ".Lconditional" + preconCount}, .type = InstrType::Label}, Section::Main);
  visitStmt(s->thenStmt);

  // After executing 'thenStmt', jump to the end to avoid executing 'elseStmt'
  std::string endLabel = ".Lmain" + preconCount;
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = endLabel},.type = InstrType::Jmp}, Section::Main);

  // False condition block (else case): label and code for 'elseStmt'
  push(Instr{.var = Label{.name = elseLabel}, .type = InstrType::Label}, Section::Main);
  if (s->elseStmt) {
    visitStmt(s->elseStmt);
  }

  // End label (where both 'thenStmt' and 'elseStmt' converge)
  push(Instr{.var = Label{.name = endLabel}, .type = InstrType::Label}, Section::Main);
}

void codegen::enumDecl(Node::Stmt *stmt) {
  EnumStmt *s = static_cast<EnumStmt *>(stmt);

  // The feilds are pushed to the .data section
  if (debug) {
    // Ahh yes, the good ol' DWARF DIEs
    dwarf::useAbbrev(dwarf::DIEAbbrev::EnumType);
    dwarf::useAbbrev(dwarf::DIEAbbrev::EnumMember);
    dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
    // Push the enum over-all DIE
    // start with label 
    push(Instr{.var = Label{.name = ".L" + s->name + "_debug_type"}, .type = InstrType::Label}, Section::DIE);
    // die data
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::EnumType) +
               "\n.long .L" + s->name + "_enum_debug_type"
               "\n.byte " + std::to_string(s->file_id) + // File ID
               "\n.byte " + std::to_string(s->line) + // Line number
               "\n.byte " + std::to_string(s->pos) + // Line column
               "\n.byte 4" // each enum member is 4 bytes
               "\n.byte 7" // encoding - DW_ATE_signed is 7
               "\n.long .Llong_debug_type\n" // And yes, for some reason, this is still required.
              , Section::DIE);
    push(Instr{.var = Label{.name = ".L" + s->name + "_enum_debug_type"}, .type = InstrType::Label}, Section::DIEString);
    pushLinker(".string \"" + s->name + "\"\n", Section::DIEString);
  }
  int fieldCount = 0;
  for (std::string &field : s->fields) {
    // Turn the enum field into an assembler constant
    push(Instr{.var = LinkerDirective{.value = ".set enum_" + s->name + "_" + field + ", " + std::to_string(fieldCount++) + "\n"}, .type = InstrType::Linker}, Section::ReadonlyData);

    // Add the enum field to the global table
    variableTable.insert({field, field});

    if (debug) {
      // Push the enum member DIE
      pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::EnumMember) +
                 "\n.long .L" + s->name + "_" + field + "_debug_enum_type"
                 "\n.byte " + std::to_string(fieldCount - 1) + // Value of the enum member
                 "\n"
      , Section::DIE);
      // Push the name of the enum member
      push(Instr{.var = Label{.name = ".L" + s->name + "_" + field + "_debug_enum_type"}, .type = InstrType::Label}, Section::DIEString);
      pushLinker(".string \"" + field + "\"\n", Section::DIEString);
    }
  }

  if (debug) pushLinker("\n.byte 0\n", Section::DIE); // End of children

  // Add the enum to the global table
  variableTable.insert({s->name, ""}); // You should never refer to the enum base itself. You can only ever get its values
}

void codegen::structDecl(Node::Stmt *stmt) {
  StructStmt *s = static_cast<StructStmt *>(stmt);
  // As a declaration, this cannot have a loc directive
  //! pushDebug(s->line, stmt->file_id, s->pos);
  int size = 0;
  std::vector<StructMember> members;
  // Calculate size by adding the size of members
  for (std::pair<std::string, Node::Type *> field : s->fields) {
    int fieldSize = getByteSizeOfType(field.second);
    size += fieldSize; // Yes, even calculates the size of nested structs.
    members.push_back({field.first, {field.second, size}});
  }
  structByteSizes.insert({s->name, {size, members}});
  if (debug) {
    // Loop over fields, append as DIEs
    dwarf::useAbbrev(dwarf::DIEAbbrev::StructType);
    dwarf::useAbbrev(dwarf::DIEAbbrev::StructMember);
    dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
    dwarf::useAbbrev(dwarf::DIEAbbrev::PointerType);
    
    // Push struct type first
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::StructType) +
               "\n.long .L" + s->name + "_debug_type\n"
               ".byte " + std::to_string(s->file_id) + // File ID
               "\n.byte " + std::to_string(s->line) + // Line number
               "\n.byte " + std::to_string(s->pos) + // Line column
               "\n.byte " + std::to_string(size) + // Size of struct
               "\n",
                Section::DIE);
    // Push name
    push(Instr{.var = Label{.name = ".L" + s->name + "_debug_type"}, .type = InstrType::Label}, Section::DIEString);
    pushLinker(".string \"" + s->name + "\"\n", Section::DIEString);

    int currentByte = 0;
    for (std::pair<std::string, Node::Type *> field : s->fields) {
      // Push member DIE
      dwarf::useType(field.second);
      std::string fieldName = ".L" + s->name + "_" + field.first + "_debug_struct_type";
      pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::StructMember) +
                 "\n.long " + fieldName +
                 "\n.long .L" + type_to_diename(field.second) + "_debug_type"
                 "\n.byte " + std::to_string(currentByte) + // Offset in struct
                 "\n",
                  Section::DIE);
      // push name in string
      push(Instr{.var = Label{.name = fieldName}, .type = InstrType::Label}, Section::DIEString);
      pushLinker(".string \"" + field.first + "\"\n", Section::DIEString);

      // Add the byte size of this to the current byte
      currentByte += getByteSizeOfType(field.second);
    }

    // Push end of children
    pushLinker(".byte 0\n", Section::DIE);
  }
}

void codegen::print(Node::Stmt *stmt) {
  PrintStmt *print = static_cast<PrintStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}, .type = InstrType::Comment}, Section::Main);
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;
  pushDebug(print->line, stmt->file_id);

  for (Node::Expr *arg : print->args) {
    std::string argType = getUnderlying(arg->asmType);
    if (argType == "") continue;
    if (argType == "str") {
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
    } else if (argType == "int") {
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

void codegen::_return(Node::Stmt *stmt) {
  ReturnStmt *returnStmt = static_cast<ReturnStmt *>(stmt);

  pushDebug(returnStmt->line, stmt->file_id, returnStmt->pos);
  if (returnStmt->expr != nullptr) {
    // Generate return value for the function
    codegen::visitExpr(returnStmt->expr);
    if (isEntryPoint) {
      popToRegister("%rdi");
      handleExitSyscall();
    } else {
      popToRegister("%rax");
      handleReturnCleanup();
    }
    return;
  }
  if (isEntryPoint) {
    moveRegister("%rax", "$0", DataSize::Qword, DataSize::Qword);
    handleExitSyscall();
  } else {
    handleReturnCleanup(); // We don't care about rax! We're exiting. We already know that nothing is being returned therefore we know that this is ok.
  }
}

void codegen::forLoop(Node::Stmt *stmt) {
  ForStmt *s = static_cast<ForStmt *>(stmt);

  std::string preconCount = std::to_string(conditionalCount++);

  push(Instr{.var = Comment{.comment = "for loop"}, .type = InstrType::Comment}, Section::Main);

  // Create unique labels for the loop start and end
  std::string preLoopLabel = "loop_pre" + std::to_string(loopCount);
  std::string postLoopLabel = "loop_post" + std::to_string(loopCount++);
  
  // Declare the loop variable
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(s->forLoop);
  IdentExpr *assignee = static_cast<IdentExpr *>(assign->assignee);

  push(Instr{.var = Comment{.comment = "For loop variable declaration"}, .type = InstrType::Comment}, Section::Main);
  variableCount += 8;
  variableTable.insert({assignee->name, std::to_string(-variableCount) + "(%rbp)"}); // Track the variable in the stack table
  pushDebug(s->line, stmt->file_id, s->pos);
  visitExpr(assign);  // Process the initial loop assignment (e.g., i = 0)
  // Remove the last instruction!! Its a push and thats bad!
  text_section.pop_back();

  // Set loop start label
  push(Instr{.var = Label{.name = preLoopLabel}, .type = InstrType::Label}, Section::Main);
  // Evaluate the loop condition
  processBinaryExpression(static_cast<BinaryExpr *>(s->condition), preconCount, postLoopLabel, true);

  // Execute the loop body (if condition is true)
  visitStmt(s->block);  // Visit the statements inside the loop body

  // Evaluate the loop increment (e.g., i++)
  if (s->optional != nullptr) {
    visitExpr(s->optional);  // Process the loop increment if provided
    text_section.pop_back();
  }

  // Jump back to the loop start
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = preLoopLabel}, .type = InstrType::Jmp}, Section::Main);

  // Set loop end label
  push(Instr{.var = Label{.name = postLoopLabel}, .type = InstrType::Label}, Section::Main);

  // Pop the loop variable from the stack
  variableTable.erase(assignee->name);
  stackSize -= 8;
};

void codegen::whileLoop(Node::Stmt *stmt) {
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "While loops not implemented!" << std::endl;
  exit(-1);
  /*
  WhileStmt *s = static_cast<WhileStmt *>(stmt);
  loopDepth++;
  pushDebug(s->line, stmt->file_id, s->pos);
  // Do something
  loopDepth--;
  */
};

void codegen::_break(Node::Stmt *stmt) {
  BreakStmt *s = static_cast<BreakStmt *>(stmt);
  
  push(Instr{.var = Comment{.comment = "break statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Jump to the end of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = ".Lloop_post" + std::to_string(loopCount - 1)}, .type = InstrType::Jmp}, Section::Main);

  // Break statements are only valid inside loops
  if (loopDepth < 1) {
    std::cerr << "Error: Break statement outside of loop" << std::endl;
    exit(-1);
  }
};

void codegen::_continue(Node::Stmt *stmt) {
  ContinueStmt *s = static_cast<ContinueStmt *>(stmt);
  
  push(Instr{.var = Comment{.comment = "continue statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Jump back to the start of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = ".Lloop_pre" + std::to_string(loopCount - 1)}, .type = InstrType::Jmp}, Section::Main);

  // Continue statements are only valid inside loops
  if (loopDepth < 1) {
    std::cerr << "Error: Continue statement outside of loop" << std::endl;
    exit(-1);
  }
};

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

// Structname passed by the varStmt's "type" field
void codegen::declareStructVariable(Node::Expr *expr, std::string structName, int whereToPut) {
  StructExpr *s = static_cast<StructExpr *>(expr);
  // At the end, we are gonna load the effective address into
  // garbage register and put that into the variable table.
  // (This is because structs cannot be passed by value, duh)

  // Make sure that the fields unordered_map is not empty
  if (structByteSizes[structName].first == 0 || s->values.size() == 0) {
    std::cout << std::to_string(structByteSizes[structName].first) << ", " << std::to_string(s->values.size()) << std::endl;
    std::cerr << "Empty struct not allowed!" << std::endl;
    exit(-1);
  }

  // The fields of the expression might be out of order from which they are defined
  // in the struct. We need to reorder them.
  std::vector<std::pair<std::string, Node::Expr *>> orderedFields;
  for (int i = 0; i < structByteSizes[structName].second.size(); i++) {
    for (std::pair<Node::Expr *, Node::Expr *> field : s->values) {
      if (static_cast<IdentExpr *>(field.first)->name
         == structByteSizes[structName].second.at(i).first) {
        orderedFields.push_back({structByteSizes[structName].second.at(i).first, field.second});
        break;
      }
    }
  }

  int structBase = whereToPut;
  // Evaluate the orderedFields and store them in the struct!!!!
  for (int i = 0; i < orderedFields.size(); i++) {
    std::pair<std::string, Node::Expr *> field = orderedFields.at(i);
    // Check if the field is a struct itself
    Node::Type *fieldType = structByteSizes[structName].second[i].second.first;
    if (fieldType->kind == ND_SYMBOL_TYPE && (structByteSizes.find(getUnderlying(fieldType)) != structByteSizes.end())) {
      // It's of type struct!
      if (field.second->kind == ND_STRUCT) {
        /*
        have a: Struct = {
          inner: {
            # Nested struct literal expressions
          }
        }
        */
        int offset = 0;
        if (i != 0) for (int j = 0; j < i; j++) {
          offset += structByteSizes[structName].second.at(j).second.second;
        }
        // Avoid doing extra work even though we are literally recursioning
        declareStructVariable(field.second, getUnderlying(fieldType), whereToPut + offset);
        continue;
      }
      // x = {
      //   a: OtherStructThatAlreadyExists;
      // }
      if (field.second->kind == ND_IDENT) {
        // Load each individual field of the ident struct into this new struct
        // This is a bit of a hack, but it works.
        IdentExpr *ident = static_cast<IdentExpr *>(field.second);
        std::string where = variableTable[ident->name];
        int structFieldSize = structByteSizes[structName].second.at(i).second.second;
        // Get the offset - this will be the sum of all previous fieldSizes
        int offset = 0;
        if (i != 0) for (int j = 0; j < i; j++) {
          offset += structByteSizes[structName].second.at(j).second.second;
        }
        int fieldVarOffset = std::stoi(where.substr(0, where.find('(')));
        // Put the identifer address into rcx
        visitExpr(ident);
        popToRegister("%rcx");

        // Go through each field of the field
        // and basically move the value into the new one
        for (int j = 0; j < structByteSizes[getUnderlying(fieldType)].second.size(); j++) {
          push(Instr{.var=Comment{.comment="Move field '" + structByteSizes[getUnderlying(fieldType)].second[j].first + 
            "' of " + ident->name + " to field '" + structByteSizes[structName].second.at(i).first + "' of '" + structName + "'"}, .type=InstrType::Comment}, Section::Main);
          // From - relative to rcx, the base of the identifier struct
          std::string from = std::to_string(-(fieldVarOffset + structByteSizes[getUnderlying(fieldType)].second[j].second.second)) + "(%rcx)";
          // To - relative to rbp, PLUS the base of the new struct
          int subFieldOffset = 0;
          if (j != 0) for (int k = 0; k < j; k++) {
            // Don't ask. Please, this is so bad...
            subFieldOffset += structByteSizes[getUnderlying(structByteSizes[structName].second[i].second.first)].second[i].second.second;
          }
          std::string to = std::to_string(-(whereToPut+subFieldOffset)) + "(%rbp)";
          pushRegister(from);
          popToRegister(to);

          // Optimizer will handle the intermediate motions of the values
        }
        continue; // Do not just push the address here :)
      }
    }
    // Evaluate the expression
    visitExpr(field.second);
    int offset = i == 0 ? 0 : structByteSizes[structName].second.at(i - 1).second.second;
    // Pop the value into a register
    popToRegister(std::to_string(-(structBase + offset)) + "(%rbp)");
  }
}

void codegen::declareArrayVariable(Node::Expr *expr, short int arrayLength, std::string varName) {
  if (expr->kind == ND_ARRAY_AUTO_FILL) {
    // This is an implicit shorthand version of setting an array to [0, 0, 0, 0, ....]
    ArrayAutoFill *s = static_cast<ArrayAutoFill *>(expr);
    if (arrayLength < 1) {
      std::string msg = "Auto-fill arrays must have an explicitly-defined length."; // the tc should catch this, but just in case
      handleError(s->line, s->pos, msg, "Codegen Error", true); // overload not exist
      ErrorClass::printError(); // replace with real code lmao
      Exit(ExitValue::GENERATOR_ERROR);
    } 
    push(Instr{.var=LeaInstr{.size = DataSize::Qword, .dest="%rdi", .src="-" + std::to_string(variableCount) + "(%rbp)"}, .type=InstrType::Lea}, Section::Main);
    moveRegister("%rcx", "$" + std::to_string(arrayLength), DataSize::Qword, DataSize::Qword);
    push(Instr{.var=XorInstr{.lhs="%rax", .rhs="%rax"}, .type=InstrType::Xor}, Section::Main);
    pushLinker("rep stosq\n\t", Section::Main); // Ah yes, the good ol' "req stosq"... (what the fuck is this thing again??)
    return;
  }

  ArrayExpr *s = static_cast<ArrayExpr *>(expr);
  int underlyingByteSize = getByteSizeOfType(s->type);
  dwarf::useType(s->type);

  int arrayBase = variableCount;
  // Evaluate the orderedFields and store them in the struct!!!!
  for (int i = 0; i < s->elements.size(); i++) {
    Node::Expr *element = s->elements.at(i);
    // Evaluate the expression
    visitExpr(element);
    // Pop the value into a register
    popToRegister(std::to_string(-(arrayBase + i * underlyingByteSize)) + "(%rbp)");
  }
  variableCount += s->elements.size() * underlyingByteSize;
  // Offset rsp
  push(Instr{.var = SubInstr{.lhs = "%rsp", .rhs = "$" + std::to_string(s->elements.size() * underlyingByteSize)}, .type = InstrType::Sub}, Section::Main);
  // Add the struct to the variable table
  variableTable.insert({varName, std::to_string(-arrayBase) + "(%rbp)"});
}