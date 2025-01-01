#include "../helper/error/error.hpp"  
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"
#include "../common.hpp"
#include "gen.hpp"

#include <iostream>
#include <string>
#include <sys/cdefs.h>

void codegen::visitStmt(Node::Stmt *stmt) {
  // compiler optimize the statement
  Node::Stmt *realStmt = CompileOptimizer::optimizeStmt(stmt);
  if (realStmt == nullptr) return; // the statement was optimized out, it was redundant
  StmtHandler handler = lookup(stmtHandlers, realStmt->kind);
  if (handler) {
    handler(realStmt);
  }
}

void codegen::expr(Node::Stmt *stmt) {
  ExprStmt *s = static_cast<ExprStmt *>(stmt);
  // Just evaluate the expression
  codegen::visitExpr(s->expr);
  text_section.pop_back(); // Get rid of the push that may follow
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

  isEntryPoint = (s->name == "main" && insideStructName == "") ? true : false;
  std::string funcName = (isEntryPoint) ? "main" : (insideStructName != "") ? "usrstruct_" + insideStructName + "_" + s->name : "usr_" + s->name;


  // WOO YEAH BABY DEBUG TIME

  std::string dieLabel = ".Ldie" + std::to_string(dieCount++);
  if (debug) {
    bool isVoid = getUnderlying(s->returnType) == "void";
    if (s->params.size() > 0) {
      dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionParam); // Formal parameter
      if (isVoid) {
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionWithParamsVoid);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionWithParamsVoid) +
                  "\n.byte 1" // External - This means "is it a public function?"
                  "\n.long .L" + funcName + "_string\n"
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
        dwarf::useType(s->returnType);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionWithParams) +
                  "\n.byte 1" // External - This means "is it a public function?"
                  "\n.long .L" + funcName + "_string\n"
                  ".byte " + std::to_string(s->file_id) + // File ID
                  "\n.byte " + std::to_string(s->line) + "\n" // Line number
                  ".byte " + std::to_string(s->pos) + "\n" // Line column
                  ".long .L" + type_to_diename(s->returnType) + "_debug_type\n"// Return type (DW_AT_type)
                  ".quad " + dieLabel + "_debug_start\n" // Low pc
                  ".quad " + dieLabel + "_debug_end - " + dieLabel + "_debug_start\n" // high pc
                  ".uleb128 0x01\n" // 1 byte is gonna follow
                  ".byte 0x9c\n"
                  , Section::DIE);
      }
    } else {
      if (isVoid) {
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionNoParamsVoid);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionNoParamsVoid) +
                  "\n.byte 1"
                  "\n.long .L" + funcName + "_string\n"
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
        dwarf::useType(s->returnType);
        pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionNoParams) +
                  "\n.byte 1"
                  "\n.long .L" + funcName + "_string\n"
                  ".byte " + std::to_string(s->file_id) + // File ID
                  "\n.byte " + std::to_string(s->line) + "\n" // Line number
                  ".byte " + std::to_string(s->pos) + "\n" // Line column
                  ".long .L" + type_to_diename(s->returnType) + "_debug_type\n" // Return type (DW_AT_type)
                  ".quad " + dieLabel + "_debug_start\n" // Low pc
                  ".quad " + dieLabel + "_debug_end - " + dieLabel + "_debug_start\n" // high pc
                  ".uleb128 0x01\n" // 1 byte is gonna follow
                  ".byte 0x9c\n"
                  , Section::DIE);
      }
    }
    
    dwarf::useStringP(funcName);
    push(Instr {.var = Label {.name = dieLabel + "_debug_start" }, .type = InstrType::Label }, Section::Main);
  }
  
  pushLinker("\n.type " + funcName + ", @function", Section::Main);
  pushLinker("\n.globl " + funcName + "\n", Section::Main); // All functions are global functions for now.
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},Section::Main);
  // push linker directive for the debug info (the line number)
  pushDebug(s->line, stmt->file_id, s->pos);
  push(Instr{.var=LinkerDirective{.value=".cfi_startproc\n\t"},.type=InstrType::Linker},Section::Main);

  // Define literally (do not adjust cfa for this)
  push(Instr{.var = PushInstr{.what = "%rbp", .whatSize = DataSize::Qword},.type = InstrType::Push},Section::Main);
  push(Instr{.var=LinkerDirective{.value=".cfi_def_cfa_offset 16\n\t"},.type=InstrType::Linker},Section::Main);
  push(Instr{.var = MovInstr{.dest = "%rbp",
    .src = "%rsp",
    .destSize = DataSize::Qword,
    .srcSize = DataSize::Qword},
    .type = InstrType::Mov},
  Section::Main);
  push(Instr{.var=LinkerDirective{.value=".cfi_def_cfa_register 6\n\t"},.type=InstrType::Linker},Section::Main);

  // Function args
  // Reset the variableCount first, though
  for (size_t i = 0; i < s->params.size(); i++) {
    variableCount += getByteSizeOfType(s->params.at(i).second);
  }

  for (size_t i = 0; i < s->params.size(); i++) {
    // Move the argument to the stack
    std::string where = std::to_string(-variableCount) + "(%rbp)";
    // check if argument is a float or an int
    int intArgCount = 0;
    int floatArgCount = 0;
    std::string argWhere = (getUnderlying(s->params[i].second) == "float") ? floatArgOrder[floatArgCount++] : intArgOrder[intArgCount--];
    DataSize sz;
    switch (getByteSizeOfType(s->params[i].second)) {
      case 1: sz = DataSize::Byte; break;
      case 2: sz = DataSize::Word; break;
      case 4: sz = DataSize::Dword; break;
      case 8: sz = DataSize::Qword; break;
      default: sz = DataSize::Qword; break;
    }
    moveRegister(where, argWhere, sz, sz);
    variableTable.insert({s->params.at(i).first, where});
    variableCount -= getByteSizeOfType(s->params.at(i).second);

    if (debug) {
      // Use the parameter type
      pushLinker("\n.uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionParam) +
                 "\n.byte " + std::to_string(s->file_id) +
                 "\n.byte " + std::to_string(s->line) +
                 "\n.byte " + std::to_string(s->pos) +
                 "\n.long .L" + type_to_diename(s->params.at(i).second) + "_debug_type\n" +
                 "\n.uleb128 0x1" // 1 byte is gonna follow
                 "\n.byte " + std::to_string(dwarf::argOP_regs.at(argWhere) + 80) + "\n"
                 "\n"
      , Section::DIE);
    }
  }
  for (size_t i = 0; i < s->params.size(); i++) {
    // Types have to be declared after for 2 reasons:
    // 1. The type will be declared as a member of the function
    // 2. It looks nice and probably works better anyways
    dwarf::useType(s->params.at(i).second);
  }

  // Do not push the lexical block
  dwarf::nextBlockDIE = false;
  codegen::visitStmt(s->block);
  dwarf::nextBlockDIE = true; // reset it
  
  // Check if last instruction was a "RET"
  if (text_section.back().type != InstrType::Ret) {
    // Push a ret anyway
    // Otherwise we SEGFAULTT
    popToRegister("%rbp");
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
      declareStructVariable(s->expr, getUnderlying(s->type), variableCount);
      int structSize = structByteSizes[getUnderlying(s->type)].first;
      if (declareVariablesForward) {
        variableCount += structSize;
        variableTable.insert({s->name, where});
      } else {
        variableTable.insert({s->name, where});
        variableCount -= structSize;
      }
    } else if (s->type->kind == ND_ARRAY_TYPE || s->type->kind == ND_ARRAY_AUTO_FILL) {
      declareArrayVariable(s->expr, static_cast<ArrayType *>(s->type)->constSize, s->name); // s->name so it can be inserted to variableTable, s->type so we know the byte sizes.
    } else {
      visitExpr(s->expr);
      popToRegister(where); // For values small enough to fit in a register.
      if (declareVariablesForward) {
        variableCount += getByteSizeOfType(s->type);
        variableTable.insert({s->name, where});
      } else {
        variableTable.insert({s->name, where});
        variableCount -= getByteSizeOfType(s->type);
      }
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
  // Get the type of the variable
  // (struct, array, pointer, or basic)
  std::string asmName = type_to_diename(s->type); // This handles things like _ptr's and _arr's, too!
  dwarf::useType(s->type);
  dwarf::useAbbrev(dwarf::DIEAbbrev::Variable);
  dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
  pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
            "\n.long .L" + s->name + "_string\n"
            ".byte " + std::to_string(s->file_id) + "\n" // File index
            ".byte " + std::to_string(s->line) + "\n" // Line number
            ".byte " + std::to_string(s->pos) + "\n" // Line column
            ".long .L" + asmName + "_debug_type\n" // Type - point to the DIE of the DW_TAG_base_type
            ".uleb128 " + std::to_string(1 + sizeOfLEB(whereBytes - 16)) + // Length of data in location definition - 3 bytes long
            "\n.byte 0x91\n" // DW_OP_fbreg (first byte)

            ".sleb128 " + std::to_string(whereBytes - 16) + "\n"
  , Section::DIE);

  // DIE String pointer
  dwarf::useStringP(s->name);
  dieCount++;
}

void codegen::block(Node::Stmt *stmt) {
  BlockStmt *s = static_cast<BlockStmt *>(stmt);
  // This should be handled by the IR when i get around to it though
  declareVariablesForward = false;
  if (s->varDeclTypes.size() > 0) {
    /*
    // -----  C  -----
    signed long long int x = 0;
    signed long long int y = 1;
    // ----- ASM -----
    movq $0, -16(%rbp)
    movq $1, -8(%rbp)
    // ----- IDK -----
    Notice how we went "backwards" in the stack
    from -16 to -8, rather than -8 to -16
    I'm not quite sure why C decides to declare variables on the stack this way,
    but it could be related to the calling convention (check the SYSV ABI) because syscalls that rely on struct *'s
    will fill the members of the struct backwards, too.

    Let's be real, the people who made the C compiler are WAYY smarter than I am.
    I'm just going to copy them because they probably knew what they were doing.
    */
    int vc = 0;
    for (Node::Type *t : s->varDeclTypes) {
      vc += getByteSizeOfType(t);
    }
    variableCount = vc;
  } else {
    declareVariablesForward = true;
  }
  size_t preSS = stackSize;
  size_t preVS = variableCount;
  scopes.push_back(std::pair(preSS, preVS));
  // push a .loc
  pushDebug(s->line, stmt->file_id, s->pos);
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
  std::string ifLabel = ".Lifthen" + std::to_string(conditionalCount);
  std::string elseLabel = ".Lifelse" + std::to_string(conditionalCount);
  std::string endLabel = ".Lifend" + std::to_string(conditionalCount);
  conditionalCount++;
  processComparison(s->condition, ifLabel, elseLabel);
  if (s->elseStmt != nullptr)
    push(Instr{.var = JumpInstr{.op=JumpCondition::Unconditioned, .label = elseLabel}, .type = InstrType::Jmp}, Section::Main);
  push(Instr{.var = Label{.name = ifLabel}, .type = InstrType::Label}, Section::Main);
  codegen::visitStmt(s->thenStmt);
  // jump to the end of the if
  push(Instr{.var = JumpInstr{.op=JumpCondition::Unconditioned, .label = endLabel}, .type = InstrType::Jmp}, Section::Main);
  if (s->elseStmt != nullptr) {
    push(Instr{.var = Label{.name = elseLabel}, .type = InstrType::Label}, Section::Main);
    codegen::visitStmt(s->elseStmt);
    push(Instr{.var = JumpInstr{.op=JumpCondition::Unconditioned, .label = endLabel}, .type = InstrType::Jmp}, Section::Main);
  }
  push(Instr{.var = Label{.name = endLabel}, .type = InstrType::Label}, Section::Main);
  return;
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
               "\n.long .L" + s->name + "_string"
               "\n.byte " + std::to_string(s->file_id) + // File ID
               "\n.byte " + std::to_string(s->line) + // Line number
               "\n.byte " + std::to_string(s->pos) + // Line column
               "\n.byte 4" // each enum member is 4 bytes
               "\n.byte 7" // encoding - DW_ATE_signed is 7
               "\n.long .Llong_debug_type\n" // And yes, for some reason, this is still required.
              , Section::DIE);
    dwarf::useStringP(s->name);
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
                 "\n.long .L" + field + "_string"
                 "\n.byte " + std::to_string(fieldCount - 1) + // Value of the enum member
                 "\n"
      , Section::DIE);
      // Push the name of the enum member
      dwarf::useStringP(field);
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

  // Time for inline function declarations
  std::string prevSname = insideStructName;
  for (Node::Stmt *stmt : s->stmts) {
    insideStructName += s->name;
    visitStmt(stmt);
    insideStructName = prevSname; // Nested structs are not allowed currently, but may be later.
  }
  if (debug) {
    // Loop over fields, append as DIEs
    dwarf::useAbbrev(dwarf::DIEAbbrev::StructType);
    dwarf::useAbbrev(dwarf::DIEAbbrev::StructMember);
    dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
    dwarf::useAbbrev(dwarf::DIEAbbrev::PointerType);
    
    // Push struct type first
    // push label
    push(Instr{.var = Label{.name = ".L" + s->name + "_debug_type"}, .type = InstrType::Label}, Section::DIE);
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::StructType) +
               "\n.long .L" + s->name + "_string\n"
               ".byte " + std::to_string(s->file_id) + // File ID
               "\n.byte " + std::to_string(s->line) + // Line number
               "\n.byte " + std::to_string(s->pos) + // Line column
               "\n.byte " + std::to_string(size) + // Size of struct
               "\n",
                Section::DIE);
    // Push name
    dwarf::useStringP(s->name);

    int currentByte = 0;
    for (std::pair<std::string, Node::Type *> field : s->fields) {
      // Push member DIE
      dwarf::useType(field.second);
      pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::StructMember) +
                 "\n.long .L" + field.first + "_string"
                 "\n.long .L" + type_to_diename(field.second) + "_debug_type"
                 "\n.byte " + std::to_string(currentByte) + // Offset in struct
                 "\n",
                  Section::DIE);
      // push name in string
      dwarf::useStringP(field.first);

      // Add the byte size of this to the current byte
      currentByte += getByteSizeOfType(field.second);
    }

    // Push end of children
    pushLinker(".byte 0\n", Section::DIE);
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
  pushDebug(s->line, stmt->file_id, s->pos);
  if (variableTable.find(assignee->name) != variableTable.end())
    // Warn, not error- but you should know that "i" will be re-declared as 0!
    handleError(assignee->line, assignee->pos, "Variable '" + assignee->name + "' already declared in this scope", "Codegen Error", false);
  variableTable.insert({assignee->name, std::to_string(-variableCount) + "(%rbp)"}); // Track the variable in the stack table
  // Push a variable declaration for the loop variable
  if (debug) {
    dwarf::useAbbrev(dwarf::DIEAbbrev::Variable);
    dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
    // we know the type is always int
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
               "\n.long .L" + assignee->name + "_string\n"
               "\n.byte " + std::to_string(s->file_id) + // File ID
               "\n.byte " + std::to_string(s->line) + // Line number
               "\n.byte " + std::to_string(s->pos) + // Line column
               "\n.long .Lint_debug_type\n" // Type - point to the DIE of the DW_TAG_base_type
               "\n.uleb128 " + std::to_string(1 + sizeOfLEB(-variableCount - 16)) + // 1 byte is gonna follow
               "\n.byte 0x91\n" // DW_OP_fbreg (first byte)
               "\n.sleb128 " + std::to_string(-variableCount - 16) + "\n",
              Section::DIE);
    // Push the name of the variable
    dwarf::useStringP(assignee->name);
  }
  if (declareVariablesForward) variableCount += 8; else variableCount -= 8;
  visitExpr(assign);  // Process the initial loop assignment (e.g., i = 0)
  // Remove the last instruction!! Its a push and thats bad!
  if (text_section[text_section.size() - 1].type == InstrType::Push)
    text_section.pop_back();

  // Set loop start label
  push(Instr{.var = Label{.name = preLoopLabel}, .type = InstrType::Label}, Section::Main);
  // Evaluate the loop condition
  visitExpr(s->condition);  // Process the loop condition
  popToRegister("%rcx");  // Pop the condition value to a register
  pushLinker("testq %rcx, %rcx\n\t", Section::Main);  // Test the condition value (not compare- we want to know the Zero flag in this case)
  // Jump to the end of the loop if the condition is false
  push(Instr{.var = JumpInstr{.op = JumpCondition::Zero, .label = postLoopLabel}, .type = InstrType::Jmp}, Section::Main);

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
  if (declareVariablesForward) variableCount -= 8; else variableCount += 8; // Undo the variable declaration- leave more room for more variables
};

void codegen::whileLoop(Node::Stmt *stmt) {
  WhileStmt *s = static_cast<WhileStmt *>(stmt);
  loopDepth++;
  pushDebug(s->line, stmt->file_id, s->pos);
  // Do basically the same thing as a for loop. No variable declaration, though.
  std::string preLoop = ".Lwhile_pre" + std::to_string(loopCount);
  std::string postLoop = ".Lwhile_post" + std::to_string(loopCount);
  loopCount++;
  // Evaluate condition
  push(Instr{.var = Label{.name = preLoop}, .type = InstrType::Label}, Section::Main);
  processComparison(s->condition, "", postLoop);
  // yummers, now just do the block
  visitStmt(s->block);

  // Eval the optional ': ()' part
  if (s->optional != nullptr) {
    visitExpr(s->optional);
    text_section.pop_back();
  }

  // Jump back to the start of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = preLoop}, .type = InstrType::Jmp}, Section::Main);
  push(Instr{.var = Label{.name = postLoop}, .type = InstrType::Label}, Section::Main);
  loopDepth--;
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

void codegen::matchStmt(Node::Stmt *stmt) {
  MatchStmt *s = static_cast<MatchStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "match statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);
  if (s->cases.size() == 0 && s->defaultCase != nullptr) {
    // Always jump to the default. What the hell are you using a switch for, anyway?
    visitStmt(s->defaultCase);
    return;
  }
  if (s->cases.size() == 0 && s->defaultCase == nullptr) {
    // Empty switch. Do not do anything or emit any code.
    return;
  }

  // Evaluate the match expression
  visitExpr(s->coverExpr);
  // Pop the value somewhere where a comp can be made
  popToRegister("%rax");

  std::string matchEndWhere = ".Lmatch_end" + std::to_string(conditionalCount + s->cases.size());
  std::string matchDefaultWhere = ".Lmatch_default" + std::to_string(conditionalCount + s->cases.size());

  // Go through each case and evaluate the condition.
  for (int i = 0; i < s->cases.size(); i++) {
    std::pair<Node::Expr *, Node::Stmt *> matchCase = s->cases[i];
    visitExpr(matchCase.first);
    // We can optimize a little further and remove the previous push and compare to its value directly!
    PushInstr prevPush = std::get<PushInstr>(text_section[text_section.size() - 1].var);
    text_section.pop_back();

    push(Instr {.var = CmpInstr {.lhs = "%rax", .rhs = prevPush.what}, .type = InstrType::Cmp}, Section::Main);
    // Jump if equal to the case
    push(Instr{.var = JumpInstr{.op = JumpCondition::Equal, .label = ".Lmatch_case" + std::to_string(conditionalCount + i)}, .type = InstrType::Jmp}, Section::Main);
  };

  // If there is a default case, jump to it. We have clearly gotten this far and not found a match.
  if (s->defaultCase == nullptr) {
    // Jump to the end of the match statement. One will always be added later.
    push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = matchEndWhere}, .type = InstrType::Jmp}, Section::Main);
  } else {
    push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = matchDefaultWhere}, .type = InstrType::Jmp}, Section::Main);
  }

  // Evaluate each case's label and, of course, statements.
  for (int i = 0; i < s->cases.size(); i++) {
    std::pair<Node::Expr *, Node::Stmt *> matchCase = s->cases[i];
    push(Instr{.var = Label{.name = ".Lmatch_case" + std::to_string(conditionalCount + i)}, .type = InstrType::Label}, Section::Main);
    visitStmt(matchCase.second);
    // Only jump to the end if there is a break. Otherwise, fall through!
    // Yes, this is intended. If you made a mistake and forgot to break, then we are not cleaning up after you!
  }

  // Evaluate the default case, if it exists
  if (s->defaultCase != nullptr) {
    push(Instr{.var = Label{.name = matchDefaultWhere}, .type = InstrType::Label}, Section::Main);
    visitStmt(s->defaultCase);
    // No need to jump here. Since we evaluated this last, we can simply fall through to the end.
  };

  // We're done! Simply append the finishing label and we will be good to go.
  push(Instr{.var = Label{.name = matchEndWhere}, .type = InstrType::Label}, Section::Main);
  conditionalCount += s->cases.size();
};

// Structname passed by the varStmt's "type" field
void codegen::declareStructVariable(Node::Expr *expr, std::string structName, int whereToPut) {
  if (expr->kind != ND_STRUCT) {
    IndexExpr *index = static_cast<IndexExpr *>(expr);
    // copy each value from the ident's struct to the new struct
    visitExpr(expr);
    PushInstr prevPush = std::get<PushInstr>(text_section[text_section.size() - 1].var);
    text_section.pop_back();
    std::string where = prevPush.what;
    push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rcx",.src=where}, .type=InstrType::Lea}, Section::Main);
    int structSize = structByteSizes[structName].first;
    for (int i = 0; i < structByteSizes[structName].first; i++) {
      // move, byte for byte, the value from the ident's struct to the new struct
      push(Instr{.var=PushInstr{.what=std::to_string(-i)+"(%rcx)",.whatSize=DataSize::Byte},.type=InstrType::Push}, Section::Main);
      push(Instr{.var=PopInstr{.where=std::to_string(whereToPut-i)+"(%rbp)",.whereSize=DataSize::Byte},.type=InstrType::Pop}, Section::Main);
    }
    return;
  }
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
        int offset;
        // Go backwards
        if (declareVariablesForward) {
          offset = 0;
          for (int j = 0; j < i; j++) {
            offset += structByteSizes[structName].second.at(j).second.second;
          }
        } else {
          offset = structByteSizes[structName].second.at(i).second.second;
          for (int j = i; j >= 0; j--) {
            offset -= structByteSizes[structName].second.at(j).second.second;
          }
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
        int offset;
        if (declareVariablesForward) {
          offset = 0;
          for (int j = 0; j < i; j++) {
            offset += structByteSizes[structName].second.at(j).second.second;
          }
        } else {
          offset = structByteSizes[structName].second.at(i).second.second;
          for (int j = i; j >= 0; j--) {
            offset -= structByteSizes[structName].second.at(j).second.second;
          }
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
          int subFieldOffset;
          if (declareVariablesForward) {
            subFieldOffset = 0;
            if (j != 0) for (int k = 0; k < j; k++) {
              // Don't ask. Please, this is so bad...
              subFieldOffset += structByteSizes[getUnderlying(structByteSizes[structName].second[i].second.first)].second[i].second.second;
            }
          } else {
            subFieldOffset = structByteSizes[structName].second[i].second.second;
            if (j != 0) for (int k = j; k >= 0; k--) {
              subFieldOffset -= structByteSizes[structName].second[i].second.second;
            }
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
    int offset = structByteSizes[structName].first;
    // Go BACKWARDS through the fields
    // if i == 0, then offset should be 0
    if (declareVariablesForward) {
      offset = 0;
      for (int j = 0; j < i; j++) {
        offset += structByteSizes[structName].second[j].second.second;
      }
      popToRegister(std::to_string(-(structBase + offset)) + "(%rbp)");
    } else {
      offset = structByteSizes[structName].second[i].second.second;
      for (int j = i; j >= 0; j--) {
        offset -= structByteSizes[structName].second[j].second.second;
      }
      popToRegister(std::to_string((-structBase - offset)) + "(%rbp)");
    }
    // Pop the value into a register
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
    // TODO: This code will no longer work after this backwards-variable thing
    push(Instr{.var=LeaInstr{.size = DataSize::Qword, .dest="%rdi", .src=std::to_string(-variableCount) + "(%rbp)"}, .type=InstrType::Lea}, Section::Main);
    moveRegister("%rcx", "$" + std::to_string(arrayLength), DataSize::Qword, DataSize::Qword);
    push(Instr{.var=XorInstr{.lhs="%rax", .rhs="%rax"}, .type=InstrType::Xor}, Section::Main);
    pushLinker("rep stosq\n\t", Section::Main); // Ah yes, the good ol' "req stosq"... (what the fuck is this thing again??)
    return;
  }

  ArrayExpr *s = static_cast<ArrayExpr *>(expr);
  int underlyingByteSize = getByteSizeOfType(static_cast<ArrayType *>(s->type)->underlying);
  dwarf::useType(s->type);

  int arrayBase = variableCount;
  // Evaluate the orderedFields and store them in the struct!!!!
  for (int i = 0; i < s->elements.size(); i++) {
    Node::Expr *element = s->elements.at(i);
    // Evaluate the expression
    if (element->kind == ND_STRUCT) {
      // Structs cannot be generated in the expr.
      // We must assign each value to its place in the array.
      ArrayType *at = static_cast<ArrayType *>(s->type);
      int startingPoint = arrayBase - (i * getByteSizeOfType(at->underlying));
      declareStructVariable(element, getUnderlying(at), startingPoint);
      continue;
    }
    visitExpr(element);
    // Pop the value into a register
    if (declareVariablesForward) {
      popToRegister(std::to_string(-(arrayBase + i * underlyingByteSize)) + "(%rbp)");
    } else {
      popToRegister(std::to_string(-(arrayBase - i * underlyingByteSize)) + "(%rbp)");
    }
  }
  // Add the struct to the variable table
  if (declareVariablesForward) {
    variableCount += s->elements.size() * underlyingByteSize;
    variableTable.insert({varName, std::to_string(-arrayBase) + "(%rbp)"});
  } else {
    variableTable.insert({varName, std::to_string(-arrayBase) + "(%rbp)"});
    variableCount -= s->elements.size() * underlyingByteSize;
  }
}