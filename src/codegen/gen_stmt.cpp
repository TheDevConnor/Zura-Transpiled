#include "../common.hpp"
#include "../helper/error/error.hpp"
#include "gen.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"

#include <iostream>
#include <string>
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
  // size_t preStackSize = stackSize;

  isEntryPoint = (s->name == "main" && insideStructName == "") ? true : false;
  std::string funcName = (isEntryPoint) ? "main"
                         : (insideStructName != "")
                             ? "usrstruct_" + insideStructName + "_" + s->name
                             : "usr_" + s->name;

  // WOO YEAH BABY DEBUG TIME

  std::string dieLabel = ".Ldie" + std::to_string(dieCount++);
  if (debug) {
    bool isVoid = getUnderlying(s->returnType) == "void";
    if (s->params.size() > 0) {
      dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionParam); // Formal parameter
      if (isVoid) {
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionWithParamsVoid);
        pushLinker(
            ".uleb128 " +
                std::to_string((int)dwarf::DIEAbbrev::FunctionWithParamsVoid) +
                "\n.byte 1" // External - This means "is it a public function?"
                "\n.long .L" +
                funcName +
                "_string\n"
                ".byte " +
                std::to_string(s->file_id) + // File ID
                "\n.byte " + std::to_string(s->line) +
                "\n" // Line number
                ".byte " +
                std::to_string(s->pos) +
                "\n" // Line column
                ".quad " +
                dieLabel +
                "_debug_start\n" // Low pc
                ".quad " +
                dieLabel + "_debug_end - " + dieLabel +
                "_debug_start\n"  // high pc
                ".uleb128 0x01\n" // 1 byte is gonna follow
                ".byte 0x9c\n",
            Section::DIE);
      } else {
        dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionWithParams);
        dwarf::useType(s->returnType);
        pushLinker(
            ".uleb128 " +
                std::to_string((int)dwarf::DIEAbbrev::FunctionWithParams) +
                "\n.byte 1" // External - This means "is it a public function?"
                "\n.long .L" +
                funcName +
                "_string\n"
                ".byte " +
                std::to_string(s->file_id) + // File ID
                "\n.byte " + std::to_string(s->line) +
                "\n" // Line number
                ".byte " +
                std::to_string(s->pos) +
                "\n" // Line column
                ".long .L" +
                type_to_diename(s->returnType) +
                "_debug_type\n" // Return type (DW_AT_type)
                ".quad " +
                dieLabel +
                "_debug_start\n" // Low pc
                ".quad " +
                dieLabel + "_debug_end - " + dieLabel +
                "_debug_start\n"  // high pc
                ".uleb128 0x01\n" // 1 byte is gonna follow
                ".byte 0x9c\n",
            Section::DIE);
      }
    } else {
      if (isVoid) {
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionNoParamsVoid);
        pushLinker(
            ".uleb128 " +
                std::to_string((int)dwarf::DIEAbbrev::FunctionNoParamsVoid) +
                "\n.byte 1"
                "\n.long .L" +
                funcName +
                "_string\n"
                ".byte " +
                std::to_string(s->file_id) + // File ID
                "\n.byte " + std::to_string(s->line) +
                "\n" // Line number
                ".byte " +
                std::to_string(s->pos) +
                "\n" // Line column
                ".quad " +
                dieLabel +
                "_debug_start\n" // Low pc
                ".quad " +
                dieLabel + "_debug_end - " + dieLabel +
                "_debug_start\n"  // high pc
                ".uleb128 0x01\n" // 1 byte is gonna follow
                ".byte 0x9c\n",
            Section::DIE);
      } else {
        dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
        dwarf::useAbbrev(dwarf::DIEAbbrev::FunctionNoParams);
        dwarf::useType(s->returnType);
        pushLinker(".uleb128 " +
                       std::to_string((int)dwarf::DIEAbbrev::FunctionNoParams) +
                       "\n.byte 1"
                       "\n.long .L" +
                       funcName +
                       "_string\n"
                       ".byte " +
                       std::to_string(s->file_id) + // File ID
                       "\n.byte " + std::to_string(s->line) +
                       "\n" // Line number
                       ".byte " +
                       std::to_string(s->pos) +
                       "\n" // Line column
                       ".long .L" +
                       type_to_diename(s->returnType) +
                       "_debug_type\n" // Return type (DW_AT_type)
                       ".quad " +
                       dieLabel +
                       "_debug_start\n" // Low pc
                       ".quad " +
                       dieLabel + "_debug_end - " + dieLabel +
                       "_debug_start\n"  // high pc
                       ".uleb128 0x01\n" // 1 byte is gonna follow
                       ".byte 0x9c\n",
                   Section::DIE);
      }
    }

    dwarf::useStringP(funcName);
    push(Instr{.var = Label{.name = dieLabel + "_debug_start"},
               .type = InstrType::Label},
         Section::Main);
  }

  pushLinker("\n.type " + funcName + ", @function", Section::Main);
  pushLinker("\n.globl " + funcName + "\n",
             Section::Main); // All functions are global functions for now.
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},
       Section::Main);
  // push linker directive for the debug info (the line number)
  pushDebug(s->line, stmt->file_id, s->pos);
  push(Instr{.var = LinkerDirective{.value = ".cfi_startproc\n\t"},
             .type = InstrType::Linker},
       Section::Main);

  pushLinker("endbr64\n\t", Section::Main); //? This instruction is really stupid and complicated, but it boils down to this:
                                            //- This instruction is necessary for function pointers because when you jump some
                                            //- stupid shit happens. Also, C does it.

  // Define literally (do not adjust cfa for this)
  push(Instr{.var = PushInstr{.what = "%rbp", .whatSize = DataSize::Qword},
             .type = InstrType::Push},
       Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".cfi_def_cfa_offset 16\n\t.cfi_offset 6, -16\n\t"},
             .type = InstrType::Linker},
       Section::Main);
  push(Instr{.var = MovInstr{.dest = "%rbp",
                             .src = "%rsp",
                             .destSize = DataSize::Qword,
                             .srcSize = DataSize::Qword},
             .type = InstrType::Mov},
       Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".cfi_def_cfa_register 6\n\t"},
             .type = InstrType::Linker},
       Section::Main);

  // Function args
  // Reset the variableCount first, though
  int preVC = variableCount;
  variableCount = 8;
  int intArgCount = 0;
  int floatArgCount = 0;
  for (size_t i = 0; i < s->params.size(); i++) {
    // Move the argument to the stack
    std::string where = std::to_string(-variableCount) + "(%rbp)";
    if (getUnderlying(s->params.at(i).second) == "float")
      // Floats are passed in xmm registers
      moveRegister(where, floatArgOrder[floatArgCount++], DataSize::Qword,
                   DataSize::Qword);
    else
      // Integers are passed in general purpose registers
      moveRegister(where, intArgOrder[intArgCount++], DataSize::Qword, DataSize::Qword);
      
    variableTable.insert({s->params.at(i).first, where});
    variableCount += getByteSizeOfType(s->params.at(i).second);

    if (debug) {
      // Use the parameter type
      pushLinker("\n.uleb128 " +
                     std::to_string((int)dwarf::DIEAbbrev::FunctionParam) +
                     "\n.byte " + std::to_string(s->file_id) + "\n.byte " +
                     std::to_string(s->line) + "\n.byte " +
                     std::to_string(s->pos) + "\n.long .L" +
                     type_to_diename(s->params.at(i).second) + "_debug_type\n" +
                     "\n.uleb128 0x1" // 1 byte is gonna follow
                     "\n.byte " +
                     std::to_string(dwarf::argOP_regs.at(intArgOrder.at(i)) + 80) +
                     "\n"
                     "\n",
                 Section::DIE);
    }
  }
  for (size_t i = 0; i < s->params.size(); i++) {
    // Types have to be declared after for 2 reasons:
    // 1. The type will be declared as a member of the function
    // 2. It looks nice and probably works better anyways
    dwarf::useType(s->params.at(i).second);
  }

  // Do not push the lexical block to the dwarf stack
  dwarf::nextBlockDIE = false;
  codegen::visitStmt(s->block);
  dwarf::nextBlockDIE = true; // reset it
  variableCount = preVC;

  // Check if last instruction was a "RET"
  if (text_section.back().type != InstrType::Ret) {
    // Push a ret anyway
    // Otherwise we SEGFAULTT
    popToRegister("%rbp");
    pushLinker(".cfi_def_cfa 7, 8\n\t", Section::Main);
    push(Instr{.var = Ret{.fromWhere = funcName}, .type = InstrType::Ret},
         Section::Main);
  }

  // Function ends with ret so we can't really push any other instructions.
  if (debug) {
    pushLinker(dieLabel + "_debug_end:\n", Section::Main);
    pushLinker(".byte 0\n", Section::DIE); // End of children
  }

  push(Instr{.var = LinkerDirective{.value = ".cfi_endproc\n"},
             .type = InstrType::Linker},
       Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" +
                                             funcName + "\n\t"},
             .type = InstrType::Linker},
       Section::Main);
  // Remove the function variables from the variable table
  for (size_t i = 0; i < s->params.size(); i++) {
    variableTable.erase(s->params.at(i).first);
  }
};

void codegen::varDecl(Node::Stmt *stmt) {
  VarStmt *s = static_cast<VarStmt *>(stmt);

  push(Instr{.var = Comment{.comment =
                                "Variable declaration for '" + s->name + "'"},
             .type = InstrType::Comment},
       Section::Main);

  // push another .loc
  pushDebug(s->line, stmt->file_id, s->pos);

  int whereBytes = -variableCount;
  std::string where = std::to_string(whereBytes) + "(%rbp)";
  if (s->expr != nullptr) {
    // The first clause: If it's a literal, straight up, struct declaration.
    // It might be a pointer to struct declaration, but the underlyingType func
    // makes it seem like a normal one instead.
    if (s->type->kind == ND_SYMBOL_TYPE &&
        structByteSizes.find(getUnderlying(s->type)) != structByteSizes.end()) {
      // It's of type struct!
      // Basically ignore the part where we allocate memory for this thing.
      declareStructVariable(s->expr, getUnderlying(s->type), "%rbp", variableCount);
      int structSize = structByteSizes[getUnderlying(s->type)].first;
      variableTable.insert({s->name, where});
      variableCount += structSize;
    } else if (s->type->kind == ND_ARRAY_TYPE ||
               s->type->kind == ND_ARRAY_AUTO_FILL) {
      ArrayType *at = static_cast<ArrayType *>(s->type);
      declareArrayVariable(
          s->expr, static_cast<ArrayType *>(s->type)->constSize,
          s->name); // s->name so it can be inserted to variableTable, s->type
                    // so we know the byte sizes.
      variableCount += (getByteSizeOfType(at->underlying) * at->constSize) + 8;
    } else {
      visitExpr(s->expr);
      DataSize size = DataSize::Qword;
      switch (getByteSizeOfType(s->type)) {
        case 1:
          size = DataSize::Byte;
          break;
        case 2:
          size = DataSize::Word;
          break;
        case 4:
          size = DataSize::Dword;
          break;
        case 8:
        default:
          size = DataSize::Qword;
          break;
      }
      if (s->type->kind == ND_SYMBOL_TYPE) {
        if (getUnderlying(s->type) == "float") {
          size = DataSize::SS;
        } else if (getUnderlying(s->type) == "double") {
          size = DataSize::SD;
        }
      }
      push(Instr{.var=PopInstr{.where = where, .whereSize = size}, .type = InstrType::Pop}, Section::Main); // For values small enough to fit in a register.
      variableTable.insert({s->name, where});
      variableCount += getByteSizeOfType(s->type);
    }
  } else {
    variableTable.insert({s->name, where});      // Insert into table
    variableCount += getByteSizeOfType(s->type); // Allocation (leaving space for future variables)
  }
  // Update the symbol table with the variable's position

  push(Instr{.var = Comment{.comment = "End of variable declaration for '" +
                                       s->name + "'"},
             .type = InstrType::Comment},
       Section::Main);

  // Push DWARF DIE for variable declaration!!!!!
  if (!debug)
    return;
  // Get the type of the variable
  // (struct, array, pointer, or basic)
  std::string asmName = type_to_diename(
      s->type); // This handles things like _ptr's and _arr's, too!
  dwarf::useType(s->type);
  dwarf::useAbbrev(dwarf::DIEAbbrev::Variable);
  dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
  pushLinker(
      ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
          "\n.long .L" + s->name +
          "_string\n"
          ".byte " +
          std::to_string(s->file_id) +
          "\n" // File index
          ".byte " +
          std::to_string(s->line) +
          "\n" // Line number
          ".byte " +
          std::to_string(s->pos) +
          "\n" // Line column
          ".long .L" +
          asmName +
          "_debug_type\n" // Type - point to the DIE of the DW_TAG_base_type
          ".uleb128 " +
          std::to_string(
              1 + sizeOfLEB(whereBytes - 16)) + // Length of data in location
                                                // definition - 3 bytes long
          "\n.byte 0x91\n"                      // DW_OP_fbreg (first byte)

          ".sleb128 " +
          std::to_string(whereBytes - 16) + "\n",
      Section::DIE);

  // DIE String pointer
  dwarf::useStringP(s->name);
  dieCount++;
}

void codegen::block(Node::Stmt *stmt) {
  BlockStmt *s = static_cast<BlockStmt *>(stmt);
  // TODO: Track the number of variables and pop them off later
  // This should be handled by the IR when i get around to it though
  size_t preSS = stackSize;
  size_t preVS = variableCount;
  scopes.push_back(std::pair(preSS, preVS));
  // AHHHH DWARF STUFF ::SOB::::::::::
  // push a .loc
  pushDebug(s->line, stmt->file_id, s->pos);
  // gotta love the ++i operator bro :D
  // if this was i++, we'd be fucking dead!!!!!!! YIPEEEE
  size_t thisDieCount = dieCount++;
  if (dwarf::nextBlockDIE) {
    if (debug)
      push(Instr{.var = Label{.name = ".Ldie" + std::to_string(thisDieCount) +
                                      "_begin"},
                 .type = InstrType::Label},
           Section::Main);
    dwarf::useAbbrev(dwarf::DIEAbbrev::LexicalBlock);
    pushLinker(".uleb128 " +
                   std::to_string((int)dwarf::DIEAbbrev::LexicalBlock) +
                   "\n.byte " + std::to_string(s->file_id) + // File ID
                   "\n.byte " + std::to_string(s->line) +    // Line number
                   "\n.byte " + std::to_string(s->pos) +     // Column number
                   "\n.long .Ldie" + std::to_string(thisDieCount) +
                   "_begin" // Low pc
                   "\n.quad .Ldie" +
                   std::to_string(thisDieCount) + "_end - .Ldie" +
                   std::to_string(thisDieCount) + "_begin\n" // High pc
               ,
               Section::DIE);
  }
  for (Node::Stmt *stm : s->stmts) {
    codegen::visitStmt(stm);
  }
  if (dwarf::nextBlockDIE) {
    pushLinker(".byte 0\n", Section::DIE); // End of children nodes of the scope
    if (debug)
      push(Instr{.var = Label{.name = ".Ldie" + std::to_string(thisDieCount) +
                                      "_end"},
                 .type = InstrType::Label},
           Section::Main);
  }
  stackSize = scopes.at(scopes.size() - 1).first;
  variableCount = scopes.at(scopes.size() - 1).second;
  scopes.pop_back();
};

void codegen::ifStmt(Node::Stmt *stmt) {
  IfStmt *s = static_cast<IfStmt *>(stmt);
  std::string elseLabel = ".Lifelse" + std::to_string(conditionalCount);
  std::string endLabel = ".Lifend" + std::to_string(conditionalCount);
  conditionalCount++;
  JumpCondition jc =
      processComparison(s->condition); // This produces the comparison code for
                                       // us. We need to jump
  if (s->elseStmt == nullptr) {
    // if () {};
    push(Instr{.var = JumpInstr{.op = getOpposite(jc), .label = endLabel},
               .type = InstrType::Jmp},
         Section::Main);
    visitStmt(s->thenStmt);
    push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                                .label = endLabel},
               .type = InstrType::Jmp},
         Section::Main);
  } else {
    // if () {} else {};
    push(Instr{.var = JumpInstr{.op = getOpposite(jc), .label = elseLabel},
               .type = InstrType::Jmp},
         Section::Main);
    visitStmt(s->thenStmt);
    push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                                .label = endLabel},
               .type = InstrType::Jmp},
         Section::Main);
    push(Instr{.var = Label{.name = elseLabel}, .type = InstrType::Label},
         Section::Main);
    visitStmt(s->elseStmt);
    push(Instr{.var = Label{.name = endLabel}, .type = InstrType::Label},
         Section::Main);
  }
  // end label
  push(Instr{.var = Label{.name = endLabel}, .type = InstrType::Label},
       Section::Main);
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
    push(Instr{.var = Label{.name = ".L" + s->name + "_debug_type"},
               .type = InstrType::Label},
         Section::DIE);
    // die data
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::EnumType) +
                   "\n.long .L" + s->name +
                   "_string"
                   "\n.byte " +
                   std::to_string(s->file_id) +           // File ID
                   "\n.byte " + std::to_string(s->line) + // Line number
                   "\n.byte " + std::to_string(s->pos) +  // Line column
                   "\n.byte 4" // each enum member is 4 bytes
                   "\n.byte 7" // encoding - DW_ATE_signed is 7
                   "\n.long .Llong_debug_type\n" // And yes, for some reason,
                                                 // this is still required.
               ,
               Section::DIE);
    dwarf::useStringP(s->name);
  }
  int fieldCount = 0;
  for (std::string &field : s->fields) {
    // Turn the enum field into an assembler constant
    push(Instr{.var = LinkerDirective{.value = ".set enum_" + s->name + "_" +
                                               field + ", " +
                                               std::to_string(fieldCount++) +
                                               "\n"},
               .type = InstrType::Linker},
         Section::ReadonlyData);

    // Add the enum field to the global table
    variableTable.insert({field, field});

    if (debug) {
      // Push the enum member DIE
      pushLinker(
          ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::EnumMember) +
              "\n.long .L" + field +
              "_string"
              "\n.byte " +
              std::to_string(fieldCount - 1) + // Value of the enum member
              "\n",
          Section::DIE);
      // Push the name of the enum member
      dwarf::useStringP(field);
    }
  }

  if (debug)
    pushLinker("\n.byte 0\n", Section::DIE); // End of children

  // Add the enum to the global table
  variableTable.insert(
      {s->name, ""}); // You should never refer to the enum base itself. You can
                      // only ever get its values
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
    members.push_back({field.first, {field.second, size}}); // Place the "size" (offset) BEFORE we add to the total
    size += fieldSize; // Yes, even calculates the size of nested structs.
  }
  structByteSizes.insert({s->name, {size, members}});

  // Time for inline function declarations
  std::string prevSname = insideStructName;
  for (Node::Stmt *stmt : s->stmts) {
    insideStructName += s->name;
    visitStmt(stmt);
    insideStructName = prevSname; // Nested structs are not allowed currently,
                                  // but may be later.
  }
  if (debug) {
    // Loop over fields, append as DIEs
    dwarf::useAbbrev(dwarf::DIEAbbrev::StructType);
    dwarf::useAbbrev(dwarf::DIEAbbrev::StructMember);
    dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
    dwarf::useAbbrev(dwarf::DIEAbbrev::PointerType);

    // Push struct type first
    // push label
    push(Instr{.var = Label{.name = ".L" + s->name + "_debug_type"},
               .type = InstrType::Label},
         Section::DIE);
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::StructType) +
                   "\n.long .L" + s->name +
                   "_string\n"
                   ".byte " +
                   std::to_string(s->file_id) +           // File ID
                   "\n.byte " + std::to_string(s->line) + // Line number
                   "\n.byte " + std::to_string(s->pos) +  // Line column
                   "\n.byte " + std::to_string(size) +    // Size of struct
                   "\n",
               Section::DIE);
    // Push name
    dwarf::useStringP(s->name);

    int currentByte = 0;
    for (std::pair<std::string, Node::Type *> field : s->fields) {
      // Push member DIE
      dwarf::useType(field.second);
      pushLinker(".uleb128 " +
                     std::to_string((int)dwarf::DIEAbbrev::StructMember) +
                     "\n.long .L" + field.first +
                     "_string"
                     "\n.long .L" +
                     type_to_diename(field.second) +
                     "_debug_type"
                     "\n.byte " +
                     std::to_string(currentByte) + // Offset in struct
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
  if (returnStmt->expr == nullptr) {
    if (isEntryPoint) {
      // Entry point (Main) function is an unsigned int, meaning that it is acceptable to return as a int here
      moveRegister("%rdi", "$0", DataSize::Qword, DataSize::Qword);
      handleExitSyscall();
      return;
    } else {
      handleReturnCleanup(); // We don't care about rax! We're exiting. We already
                             // know that nothing is being returned therefore we
                             // know that this is ok.
      std::string msg ="WARNING: Returning from a non-void function without a return value. This is undefined behavior.";
      handleError(returnStmt->line, returnStmt->pos, msg, "codegen");
      return;
    }
  }
  // Generate return value for the function
  codegen::visitExpr(returnStmt->expr);
  if (isEntryPoint) {
    popToRegister("%rdi");
    handleExitSyscall();
    return;
  }
  if (returnStmt->expr->asmType->kind == ND_POINTER_TYPE ||
      returnStmt->expr->asmType->kind == ND_FUNCTION_TYPE ||
      returnStmt->expr->asmType->kind == ND_FUNCTION_TYPE_PARAM) {
    popToRegister("%rax"); // rax is fine in this case- it is ALWAYS 8 bytes
    handleReturnCleanup();
    return;
  }
  // Probably a symbol type!
  SymbolType *st = static_cast<SymbolType *>(returnStmt->expr->asmType);
  if (st->name == "float" || st->name == "double") {
    push(Instr{.var=PopInstr{.where="%xmm0",.whereSize=DataSize::SS},.type=InstrType::Pop},Section::Main);
    handleReturnCleanup();
  } else {
    int byteSize = getByteSizeOfType(returnStmt->expr->asmType);
    if (byteSize <= 8) {
      switch (byteSize) {
        case 1:
          push(Instr{.var=PopInstr{.where="%al",.whereSize=DataSize::Byte},.type=InstrType::Pop},Section::Main);
          break;
        case 2:
          push(Instr{.var=PopInstr{.where="%ax",.whereSize=DataSize::Word},.type=InstrType::Pop},Section::Main);
          break;
        case 4:
          push(Instr{.var=PopInstr{.where="%eax",.whereSize=DataSize::Dword},.type=InstrType::Pop},Section::Main);
          break;
        case 8:
          push(Instr{.var=PopInstr{.where="%rax",.whereSize=DataSize::Qword},.type=InstrType::Pop},Section::Main);
          break;
        default:
          push(Instr{.var=PopInstr{.where="%rax",.whereSize=DataSize::Qword},.type=InstrType::Pop},Section::Main);
          break;
      }
      handleReturnCleanup();
      return;
    }
    // TODO: Return things LARGER than 8 bytes!
  }
}

void codegen::forLoop(Node::Stmt *stmt) {
  ForStmt *s = static_cast<ForStmt *>(stmt);

  std::string preconCount = std::to_string(conditionalCount++);

  push(Instr{.var = Comment{.comment = "for loop"}, .type = InstrType::Comment},
       Section::Main);

  // Create unique labels for the loop start and end
  std::string preLoopLabel = "loop_pre" + std::to_string(loopCount);
  std::string postLoopLabel = "loop_post" + std::to_string(loopCount++);

  // Declare the loop variable
  AssignmentExpr *assign = static_cast<AssignmentExpr *>(s->forLoop);
  IdentExpr *assignee = static_cast<IdentExpr *>(assign->assignee);

  push(Instr{.var = Comment{.comment = "For loop variable declaration"},
             .type = InstrType::Comment},
       Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);
  // assign var
  variableTable.insert(
      {assignee->name, std::to_string(-variableCount) + "(%rbp)"});
  variableCount += 8;
  // Push a variable declaration for the loop variable
  if (debug) {
    dwarf::useAbbrev(dwarf::DIEAbbrev::Variable);
    dwarf::useAbbrev(dwarf::DIEAbbrev::Type);
    // we know the type is always int
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
                   "\n.long .L" + assignee->name +
                   "_string\n"
                   "\n.byte " +
                   std::to_string(s->file_id) +           // File ID
                   "\n.byte " + std::to_string(s->line) + // Line number
                   "\n.byte " + std::to_string(s->pos) +  // Line column
                   "\n.long .Lint_debug_type\n" // Type - point to the DIE of
                                                // the DW_TAG_base_type
                   "\n.uleb128 " +
                   std::to_string(1 + sizeOfLEB(-variableCount -
                                                16)) + // 1 byte is gonna follow
                   "\n.byte 0x91\n" // DW_OP_fbreg (first byte)
                   "\n.sleb128 " +
                   std::to_string(-variableCount - 16) + "\n",
               Section::DIE);
    // Push the name of the variable
    dwarf::useStringP(assignee->name);
  }
  visitExpr(assign); // Process the initial loop assignment (e.g., i = 0)
  // Remove the last instruction!! Its a push and thats bad!
  text_section.pop_back();

  // Set loop start label
  push(Instr{.var = Label{.name = preLoopLabel}, .type = InstrType::Label},
       Section::Main);
  // Evaluate the loop condition
  JumpCondition jc = processComparison(s->condition);
  push(Instr{.var = JumpInstr{.op = getOpposite(jc), .label = postLoopLabel},
             .type = InstrType::Jmp},
       Section::Main);

  // Execute the loop body (if condition is true)
  visitStmt(s->block); // Visit the statements inside the loop body

  // Evaluate the loop increment (e.g., i++)
  if (s->optional != nullptr) {
    visitExpr(s->optional); // Process the loop increment if provided
    text_section.pop_back();
  }

  // Jump back to the loop start
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = preLoopLabel},
             .type = InstrType::Jmp},
       Section::Main);

  // Set loop end label
  push(Instr{.var = Label{.name = postLoopLabel}, .type = InstrType::Label},
       Section::Main);

  // Pop the loop variable from the stack
  variableTable.erase(assignee->name);
  variableCount -= 8; // We now have room for another variable!
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
  push(Instr{.var = Label{.name = preLoop}, .type = InstrType::Label},
       Section::Main);
  JumpCondition jc = processComparison(s->condition);
  push(Instr{.var = JumpInstr{.op = getOpposite(jc), .label = postLoop},
             .type = InstrType::Jmp},
       Section::Main);

  // yummers, now just do the block
  visitStmt(s->block);

  // Eval the optional ': ()' part
  if (s->optional != nullptr) {
    visitExpr(s->optional);
    text_section.pop_back();
  }

  // Jump back to the start of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = preLoop},
             .type = InstrType::Jmp},
       Section::Main);
  push(Instr{.var = Label{.name = postLoop}, .type = InstrType::Label},
       Section::Main);
  loopDepth--;
};

void codegen::_break(Node::Stmt *stmt) {
  BreakStmt *s = static_cast<BreakStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "break statement"},
             .type = InstrType::Comment},
       Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Jump to the end of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = ".Lloop_post" +
                                       std::to_string(loopCount - 1)},
             .type = InstrType::Jmp},
       Section::Main);

  // Break statements are only valid inside loops
  if (loopDepth < 1) {
    std::cerr << "Error: Break statement outside of loop" << std::endl;
    exit(-1);
  }
};

void codegen::_continue(Node::Stmt *stmt) {
  ContinueStmt *s = static_cast<ContinueStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "continue statement"},
             .type = InstrType::Comment},
       Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Jump back to the start of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label =
                                  ".Lloop_pre" + std::to_string(loopCount - 1)},
             .type = InstrType::Jmp},
       Section::Main);

  // Continue statements are only valid inside loops
  if (loopDepth < 1) {
    std::cerr << "Error: Continue statement outside of loop" << std::endl;
    exit(-1);
  }
};

void codegen::matchStmt(Node::Stmt *stmt) {
  MatchStmt *s = static_cast<MatchStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "match statement"},
             .type = InstrType::Comment},
       Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);
  if (s->cases.size() == 0 && s->defaultCase != nullptr) {
    // Always jump to the default. What the hell are you using a switch for,
    // anyway?
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

  std::string matchEndWhere =
      ".Lmatch_end" + std::to_string(conditionalCount + s->cases.size());
  std::string matchDefaultWhere =
      ".Lmatch_default" + std::to_string(conditionalCount + s->cases.size());

  // Go through each case and evaluate the condition.
  for (size_t i = 0; i < s->cases.size(); i++) {
    std::pair<Node::Expr *, Node::Stmt *> matchCase = s->cases[i];
    visitExpr(matchCase.first);
    // We can optimize a little further and remove the previous push and compare
    // to its value directly!
    PushInstr prevPush =
        std::get<PushInstr>(text_section[text_section.size() - 1].var);
    text_section.pop_back();

    push(Instr{.var = CmpInstr{.lhs = "%rax", .rhs = prevPush.what},
               .type = InstrType::Cmp},
         Section::Main);
    // Jump if equal to the case
    push(Instr{.var = JumpInstr{.op = JumpCondition::Equal,
                                .label = ".Lmatch_case" +
                                         std::to_string(conditionalCount + i)},
               .type = InstrType::Jmp},
         Section::Main);
  };

  // If there is a default case, jump to it. We have clearly gotten this far and
  // not found a match.
  if (s->defaultCase == nullptr) {
    // Jump to the end of the match statement. One will always be added later.
    push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                                .label = matchEndWhere},
               .type = InstrType::Jmp},
         Section::Main);
  } else {
    push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                                .label = matchDefaultWhere},
               .type = InstrType::Jmp},
         Section::Main);
  }

  // Evaluate each case's label and, of course, statements.
  for (size_t i = 0; i < s->cases.size(); i++) {
    std::pair<Node::Expr *, Node::Stmt *> matchCase = s->cases[i];
    push(Instr{.var = Label{.name = ".Lmatch_case" +
                                    std::to_string(conditionalCount + i)},
               .type = InstrType::Label},
         Section::Main);
    visitStmt(matchCase.second);
    // Only jump to the end if there is a break. Otherwise, fall through!
    // Yes, this is intended. If you made a mistake and forgot to break, then we
    // are not cleaning up after you!
  }

  // Evaluate the default case, if it exists
  if (s->defaultCase != nullptr) {
    push(Instr{.var = Label{.name = matchDefaultWhere},
               .type = InstrType::Label},
         Section::Main);
    visitStmt(s->defaultCase);
    // No need to jump here. Since we evaluated this last, we can simply fall
    // through to the end.
  };

  // We're done! Simply append the finishing label and we will be good to go.
  push(Instr{.var = Label{.name = matchEndWhere}, .type = InstrType::Label},
       Section::Main);
  conditionalCount += s->cases.size();
};

void codegen::dereferenceStructPtr(Node::Expr *expr, std::string structName,
                                    std::string offsetRegister, int startOffset) {
  // have DerefStruct: StructName = Struct&;
  // see how large the struct is
  DereferenceExpr *deref = static_cast<DereferenceExpr *>(expr);
  int structSize = structByteSizes[structName].first;
  if (structSize > 16) {
    // we should optimize into a rep mov
    // The lhs of the deref is probably an ident
    // so we should evlauate it
    visitExpr(deref->left);
    PushInstr prevPush =
        std::get<PushInstr>(text_section[text_section.size() - 1].var);
    if (prevPush.what.find('(') == std::string::npos) popToRegister("%rsi");
    else {
      text_section.pop_back();
      // leaq
      push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rsi",.src=prevPush.what},.type=InstrType::Lea},Section::Main);
    }
    // the destination is just the offset register and the offset, lol
    push(Instr{.var=Comment{.comment="Optimized struct copy"},.type=InstrType::Comment},Section::Main);
    // lea the dest
    push(Instr{.var=LeaInstr{.size=DataSize::Qword,.dest="%rdi",.src=std::to_string(-(startOffset+structSize))+"("+offsetRegister+")"},.type=InstrType::Lea},Section::Main);
    // if divisible by 8, do qword, if 4, do dword, etc
    if (structSize % 8 == 0) {
      moveRegister("%rcx", "$" + std::to_string(structSize / 8), DataSize::Qword, DataSize::Qword);
      pushLinker("rep movsq\n\t", Section::Main);
    } else if (structSize % 4 == 0) {
      moveRegister("%rcx", "$" + std::to_string(structSize / 4), DataSize::Qword, DataSize::Qword);
      pushLinker("rep movsd\n\t", Section::Main);
    } else if (structSize % 2 == 0) {
      moveRegister("%rcx", "$" + std::to_string(structSize / 2), DataSize::Qword, DataSize::Qword);
      pushLinker("rep movsw\n\t", Section::Main);
    } else {
      moveRegister("%rcx", "$" + std::to_string(structSize), DataSize::Qword, DataSize::Qword);
      pushLinker("rep movsb\n\t", Section::Main);
    }
    return;
  }
  // The struct was pretty small, so a rep would be counterintuitive
  // so we will just copy the bytes manually
  visitExpr(deref->left);
  popToRegister("%rsi");
  while (structSize > 0) {
    if (structSize >= 8) {
      // move a qword from src to dst
      structSize -= 8;
      moveRegister("%rax", std::to_string(structSize) + "(%rsi)", DataSize::Qword, DataSize::Qword);
      moveRegister(std::to_string(-(startOffset + structSize)) + "(" + offsetRegister + ")", "%rax", DataSize::Qword, DataSize::Qword);
    } else if (structSize >= 4) {
      structSize -= 4;
      moveRegister("%eax", std::to_string(structSize) + "(%rsi)", DataSize::Dword, DataSize::Dword);
      moveRegister(std::to_string(-(startOffset + structSize)) + "(" + offsetRegister + ")", "%eax", DataSize::Dword, DataSize::Dword);
    } else if (structSize >= 2) {
      structSize -= 2;
      moveRegister("%ax", std::to_string(structSize) + "(%rsi)", DataSize::Word, DataSize::Word);
      moveRegister(std::to_string(-(startOffset + structSize)) + "(" + offsetRegister + ")", "%ax", DataSize::Word, DataSize::Word);
    } else {
      structSize -= 1;
      moveRegister("%al", std::to_string(structSize) + "(%rsi)", DataSize::Byte, DataSize::Byte);
      moveRegister(std::to_string(-(startOffset + structSize)) + "(" + offsetRegister + ")", "%al", DataSize::Byte, DataSize::Byte);
    }
  }
}

// Structname passed by the varStmt's "type" field
void codegen::declareStructVariable(Node::Expr *expr, std::string structName,
                                    std::string offsetRegister, int startOffset) {
  if (expr->kind == ND_DEREFERENCE) {
    dereferenceStructPtr(expr, structName, offsetRegister, startOffset);
    return; // We do not want to continue!
  }
  StructExpr *s = static_cast<StructExpr *>(expr);
  // The only purpose of this function is to put memory in the right place
  // it does not declare the struct in the variable table or modify the variable count

  // Order the fields in the struct
  std::vector<Node::Expr *> fields = {};
  for (StructMember field : structByteSizes[structName].second) {
    for (std::pair<std::string, Node::Expr *> f : s->values) {
      if (f.first == field.first) {
        fields.push_back(f.second);
      }
    }
  }

  // evaluate each field and store its result inside the (outer) struct
  for (size_t i = 0; i < fields.size(); i++) {
    Node::Expr *field = fields.at(i);
    unsigned short offset = structByteSizes[structName].second.at(i).second.second;
    if (field->asmType->kind == ND_SYMBOL_TYPE && structByteSizes.find(getUnderlying(field->asmType)) != structByteSizes.end()) {
      if (field->kind == ND_STRUCT) {
        // Evaluate THAT struct but place it in the outer struct at the offset
        declareStructVariable(field, getUnderlying(field->asmType), offsetRegister, startOffset + offset);
        continue;
      }
      // It must be an identifier or something, which means we must copy each byte from the original into this
      visitExpr(field);
      popToRegister("%rdx"); // Hopefully, 'field' will be something that returns the address

      // NEXT UPPP we must copy each byte
      short byteCount = getByteSizeOfType(field->asmType);
      while (byteCount > 0) {
        if (byteCount >= 8) {
          // Move a qword
          byteCount -= 8;
          moveRegister("%rax", std::to_string(-byteCount) + "(%rdx)", DataSize::Qword, DataSize::Qword);
          moveRegister(std::to_string(-(byteCount + startOffset + offset)) + "(" + offsetRegister + ")", "%rax", DataSize::Qword, DataSize::Qword);
        } else if (byteCount >= 4) {
          // Move a dword (long)
          byteCount -= 4;
          moveRegister("%eax", std::to_string(-byteCount) + "(%rdx)", DataSize::Dword, DataSize::Dword);
          moveRegister(std::to_string(-(byteCount + startOffset + offset)) + "(" + offsetRegister + ")", "%eax", DataSize::Dword, DataSize::Dword);
        } else if (byteCount >= 2) {
          // Move a word (short)
          byteCount -= 2;
          moveRegister("%ax", std::to_string(-byteCount) + "(%rdx)", DataSize::Word, DataSize::Word);
          moveRegister(std::to_string(-(byteCount + startOffset + offset)) + "(" + offsetRegister + ")", "%ax", DataSize::Word, DataSize::Word);
        } else {
          // Move a byte (char)
          byteCount -= 1;
          moveRegister("%al", std::to_string(-byteCount) + "(%rdx)", DataSize::Byte, DataSize::Byte);
          moveRegister(std::to_string(-(byteCount + startOffset + offset)) + "(" + offsetRegister + ")", "%al", DataSize::Byte, DataSize::Byte);
        }
      }
      continue;
    }

    if (getByteSizeOfType(field->asmType) > 8) {
      // How is that possible? It's not even a struct.
      handleError(s->line, s->pos, "Struct field is too large to fit in a struct", "codegen", true);
    }

    // Evaluate the field and store it in the struct
    visitExpr(field);
    switch (getByteSizeOfType(field->asmType)) {
      case 1:
        push(Instr{.var=PopInstr{.where=std::to_string(-(startOffset + offset)) + "(" + offsetRegister + ")",.whereSize=DataSize::Byte},.type=InstrType::Pop},Section::Main);
        break;
      case 2:
        push(Instr{.var=PopInstr{.where=std::to_string(-(startOffset + offset)) + "(" + offsetRegister + ")",.whereSize=DataSize::Word},.type=InstrType::Pop},Section::Main);
        break;
      case 4:
        push(Instr{.var=PopInstr{.where=std::to_string(-(startOffset + offset)) + "(" + offsetRegister + ")",.whereSize=DataSize::Dword},.type=InstrType::Pop},Section::Main);
        break;
      case 8:
      default:
        push(Instr{.var=PopInstr{.where=std::to_string(-(startOffset + offset)) + "(" + offsetRegister + ")",.whereSize=DataSize::Qword},.type=InstrType::Pop},Section::Main);
        break;
    }
    continue;
  }
}

void codegen::declareArrayVariable(Node::Expr *expr, short int arrayLength,
                                   std::string varName) {
  if (expr->kind == ND_ARRAY_AUTO_FILL) {
    // This is an implicit shorthand version of setting an array to [0, 0, 0, 0, ....]
    ArrayAutoFill *s = static_cast<ArrayAutoFill *>(expr);
    int totalSize = arrayLength * getByteSizeOfType(s->fillType);
    if (totalSize <= 256) {
      // Small enough for manual labor hehe
      // Ensure that filling happens from the top
      int maxQwords = totalSize / 8;
      // find remainder bytes later if any
      for (int i = 0; i < maxQwords; i++) {
        push(Instr{.var=MovInstr{.dest=std::to_string(-(variableCount + totalSize - (i * 8))) + "(%rbp)", .src="$0", .destSize=DataSize::Qword, .srcSize=DataSize::Qword}, .type=InstrType::Mov}, Section::Main);
      }
      if (int remainderBytes = totalSize % 8) {
        // Fill the remainder bytes
        for (int i = 0; i < remainderBytes; i++) {
          push(Instr{.var=MovInstr{.dest=std::to_string(-(variableCount + totalSize - remainderBytes + i)) + "(%rbp)", .src="$0", .destSize=DataSize::Byte, .srcSize=DataSize::Byte}, .type=InstrType::Mov}, Section::Main);
        }
      }
      variableTable.insert({varName, std::to_string(-(variableCount + totalSize)) + "(%rbp)"});
      return;
    }
    // C allocates 16 bytes near the top for some reason, let's rip them off and do the same
    // Assuming the autofill is small enough, we could manually fill them with 0's mov by mov
    int preVarCount = variableCount;
    variableCount = round(variableCount, 16);
    push(Instr{.var=MovInstr{.dest=std::to_string(-(variableCount + totalSize)) + "(%rbp)", .src="$0", .destSize=DataSize::Qword, .srcSize=DataSize::Qword}, .type=InstrType::Mov}, Section::Main);
    push(Instr{.var=MovInstr{.dest=std::to_string(-(variableCount + totalSize - 8)) + "(%rbp)", .src="$0", .destSize=DataSize::Qword, .srcSize=DataSize::Qword}, .type=InstrType::Mov}, Section::Main);
    // Prepare the registers for the rep stosq instruction
    push(Instr{.var = LeaInstr{.size = DataSize::Qword, .dest = "%rdx", .src = "-" + std::to_string(variableCount + arrayLength - 16) + "(%rbp)"}, .type = InstrType::Lea}, Section::Main);
    push(Instr{.var = XorInstr{.lhs = "%rax", .rhs = "%rax"}, .type = InstrType::Xor}, Section::Main);
    moveRegister("%rdi", "%rdx", DataSize::Qword, DataSize::Qword);
    int newSize = totalSize - 16;
    if (newSize % 8 == 0) {
      moveRegister("%rcx", "$" + std::to_string(newSize / 8), DataSize::Qword,
                  DataSize::Qword);
      pushLinker("rep stosq\n\t",Section::Main); // Repeat %rcx times to fill ptr %rdx with the value of %rax (every 8 bytes)
    } else if (newSize % 4 == 0) {
      moveRegister("%rcx", "$" + std::to_string(newSize / 4), DataSize::Qword,
                  DataSize::Qword);
      pushLinker("rep stosd\n\t",Section::Main); // Repeat %rcx times to fill ptr %rdx with the value of %rax (every 8 bytes)
    } else if (newSize % 2 == 0) {
      moveRegister("%rcx", "$" + std::to_string(newSize / 2), DataSize::Qword,
                  DataSize::Qword);
      pushLinker("rep stosw\n\t",Section::Main); // Repeat %rcx times to fill ptr %rdx with the value of %rax (every 8 bytes)
    } else {
      moveRegister("%rcx", "$" + std::to_string(newSize), DataSize::Qword,
                  DataSize::Qword);
      pushLinker("rep stosb\n\t",Section::Main); // Repeat %rcx times to fill ptr %rdx with the value of %rax (every 8 bytes)
    }
    // Add the array to the variable table
    variableTable.insert({varName, std::to_string(-(variableCount + totalSize)) + "(%rbp)"});
    variableCount = preVarCount;
    return;
  }

  ArrayExpr *s = static_cast<ArrayExpr *>(expr);
  ArrayType *at = static_cast<ArrayType *>(s->type);
  dwarf::useType(s->type);
  int underlyingByteSize =
      getByteSizeOfType(at->underlying);
  int arrayBase = variableCount;
  // Evaluate the orderedFields and store them in the struct!!!!
  for (int i = s->elements.size(); i > 0; i--) {
    Node::Expr *element = s->elements[s->elements.size() - i];
    // Evaluate the expression
    if (element->kind == ND_STRUCT) {
      // Structs cannot be generated in the expr.
      // We must assign each value to its place in the array.
      int startingPoint = arrayBase + ((i-1) * underlyingByteSize);
      declareStructVariable(element, getUnderlying(at), "%rbp", startingPoint);
      continue;
    }
    visitExpr(element);
    // Pop the value into a register
    popToRegister(std::to_string(-(arrayBase + ((i) * underlyingByteSize))) +
                  "(%rbp)");
  }

  // NOTE: The value of arrays, when referencing their variable name, is a pointer to element #1.
  // NOTE: So let's do the same calculation as earlier to find that very 1st byte.
  int firstByteOffset = -(arrayBase + ((s->elements.size()) * underlyingByteSize));
  variableTable.insert({varName, std::to_string(firstByteOffset) + "(%rbp)"});
}

void codegen::inputStmt(Node::Stmt *stmt) {
  InputStmt *s = static_cast<InputStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "input statement"}, .type = InstrType::Comment},Section::Main);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Now that we printed the "prompt", we now have to make the syscall using the values
  // do in reverse order because one of these may screw up the values of the registers
  visitExpr(s->bufferOut); // rsi (should be leaq'd)
  visitExpr(s->maxBytes);  // rdx
  visitExpr(s->fd);        // rdi
  popToRegister("%rdi");   // rdi
  popToRegister("%rdx");   // rdx
  popToRegister("%rsi");   // rsi
  // RAX and RDI are constant in this case- constant syscall number and constant file descriptor (fd of stdin is 0)
  push(Instr{.var=XorInstr{.lhs="%rax",.rhs="%rax"},.type=InstrType::Xor},Section::Main);
  push(Instr{.var=Syscall{.name="read"},.type=InstrType::Syscall},Section::Main);
}


void codegen::closeStmt(Node::Stmt *stmt) {
  CloseStmt *s = static_cast<CloseStmt *>(stmt);
  pushDebug(s->line, stmt->file_id, s->pos);

  // Now that we printed the "prompt", we now have to make the syscall using the values
  // do in reverse order because one of these may screw up the values of the registers
  visitExpr(s->fd); // rdi
  popToRegister("%rdi");   // rdi
  // RAX is constant in this case- constant syscall number
  // Stupid AI! Rax is not 0...
  moveRegister("%rax", "$3", DataSize::Qword, DataSize::Qword);
  push(Instr{.var=Syscall{.name="close"},.type=InstrType::Syscall},Section::Main);
}