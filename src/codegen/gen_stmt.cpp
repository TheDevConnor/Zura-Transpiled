#include <sys/cdefs.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "../common.hpp"
#include "../helper/error/error.hpp"
#include "gen.hpp"
#include "optimizer/compiler.hpp"
#include "optimizer/instr.hpp"

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
  if (text_section.back().type == InstrType::Push)
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
    // .cfi_sections .debug_frame
    // That tells the assembler to put the call frame information in both the .eh_frame section and the .debug_frame section
    // Im not sure if this is important. Gdb and lldb work off of the .eh_frame JUST fine. I am including this for completion's sake
    pushLinker("\n.cfi_sections .debug_frame\n\t", Section::Main);
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
    // Push the label addresses to the debug aranges
    die_arange_section.push_back({dieLabel + "_debug_start", dieLabel + "_debug_end"});
  }

  pushLinker("\n.type " + funcName + ", @function", Section::Main);
  pushLinker("\n.globl " + funcName + "\n",
             Section::Main); // All functions are global (public, linker viewable) functions for now.
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},
       Section::Main);
  // push linker directive for the debug info (the line number)
  pushDebug(s->line, stmt->file_id, s->pos);
  push(Instr{.var = LinkerDirective{.value = ".cfi_startproc\n\t"},
             .type = InstrType::Linker},
       Section::Main);

  pushLinker("endbr64\n\t",
             Section::Main); //? This instruction is really stupid and
                             // complicated, but it boils down to this:
                             //- This instruction is necessary for function
                             // pointers because when you jump some
                             //- stupid shit happens. Also, C does it.

  // Define literally (do not adjust cfa for this)
  push(Instr{.var = PushInstr{.what = "%rbp", .whatSize = DataSize::Qword},
             .type = InstrType::Push},
       Section::Main);
  push(Instr{.var =
                 LinkerDirective{
                     .value =
                         ".cfi_def_cfa_offset 16\n\t.cfi_offset %rbp, -16\n\t"},
             .type = InstrType::Linker},
       Section::Main);
  push(Instr{.var = MovInstr{.dest = "%rbp",
                             .src = "%rsp",
                             .destSize = DataSize::Qword,
                             .srcSize = DataSize::Qword},
             .type = InstrType::Mov},
       Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".cfi_def_cfa_register %rbp\n\t"},
             .type = InstrType::Linker},
       Section::Main);

  // Function args
  // Reset the variableCount first, though
  size_t preVC = variableCount;
  variableCount = 8;
  int intArgCount = 0;
  int floatArgCount = 0;
  for (size_t i = 0; i < s->params.size(); i++) {
    // Move the argument to the stack
    std::string where = std::to_string(-(long long)(variableCount)) + "(%rbp)";
    if (getUnderlying(s->params.at(i).second) == "float")
      // Floats are passed in xmm registers
      moveRegister(where, floatArgOrder[floatArgCount++], DataSize::Qword,
                   DataSize::Qword);
    else
      // Integers are passed in general purpose registers
      moveRegister(where, intArgOrder[intArgCount++], DataSize::Qword,
                   DataSize::Qword);

    variableTable.insert({s->params.at(i).first->name, where});
    variableCount += getByteSizeOfType(s->params.at(i).second);

    if (debug) {
      // Use the parameter type
      pushLinker(
          "\n.uleb128 " + std::to_string((int)dwarf::DIEAbbrev::FunctionParam) +
              "\n.byte " + std::to_string(s->file_id) + "\n.byte " +
              std::to_string(s->line) + "\n.byte " + std::to_string(s->pos) +
              "\n.long .L" + type_to_diename(s->params.at(i).second) +
              "_debug_type\n" +
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
    pushLinker(".cfi_def_cfa %rsp, 8\n\t", Section::Main);
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
    variableTable.erase(s->params.at(i).first->name);
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

  long long whereBytes = -variableCount;
  std::string where = std::to_string(whereBytes) + "(%rbp)";
  if (s->expr != nullptr) {
    // The first clause: If it's a literal, straight up, struct declaration.
    // It might be a pointer to struct declaration, but the underlyingType func
    // makes it seem like a normal one instead.
    if (s->type->kind == ND_SYMBOL_TYPE &&
        structByteSizes.contains(getUnderlying(s->type)) && 
        s->expr->kind == ND_STRUCT) {
      // It's of type struct!
      // Basically ignore the part where we allocate memory for this thing.
      declareStructVariable(s->expr, getUnderlying(s->type), "%rbp",
                            variableCount);
      int structSize = structByteSizes[getUnderlying(s->type)].first;
      variableCount += structSize;
      variableTable.insert(
        {s->name, std::to_string(-(variableCount - 8)) + "(%rbp)"});
    } else if (s->type->kind == ND_ARRAY_TYPE ||
               s->type->kind == ND_ARRAY_AUTO_FILL) {
      ArrayType *at = static_cast<ArrayType *>(s->type);

      declareArrayVariable(
          s->expr, static_cast<ArrayType *>(s->type)->constSize);
      // Insert the variable into the table
      variableCount += (getByteSizeOfType(at->underlying) * at->constSize);
      variableTable.insert(
          {s->name, std::to_string(-(variableCount - 8)) + "(%rbp)"});
    } else {
      visitExpr(s->expr);
      bool isFloatingType = s->type->kind == ND_SYMBOL_TYPE &&
                            (getUnderlying(s->type) == "float" ||
                             getUnderlying(s->type) == "double");
      DataSize size = isFloatingType
                          ? intDataToSizeFloat(getByteSizeOfType(s->type))
                          : intDataToSize(getByteSizeOfType(s->type));
      // If it was a call expression, and it returned a struct, DONT DO THIS
      if (s->expr->kind == ND_CALL && structByteSizes.contains(getUnderlying(s->type)) && getByteSizeOfType(s->type) > 8) {
        variableTable.insert({s->name, std::to_string(-variableCount + 8) + "(%rbp)"});
      } else {
        push(Instr{.var = PopInstr{.where = where, .whereSize = size},
          .type = InstrType::Pop},
             Section::Main); // For values small enough to fit in a register.
        variableTable.insert({s->name, where});
        variableCount += getByteSizeOfType(s->type);
      }
    }
  } else {
    variableTable.insert({s->name, where}); // Insert into table
    variableCount += getByteSizeOfType(
        s->type); // Allocation (leaving space for future variables)
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
  signed long long fbreg_loc = whereBytes - 16;
  // if struct, we must change this fbreg loc because of stinky reasons
  if (s->type->kind == ND_SYMBOL_TYPE &&
      structByteSizes.find(getUnderlying(s->type)) != structByteSizes.end()) {
    // Fbreg = location of last member of struct - 16
    Struct &st = structByteSizes[getUnderlying(s->type)];
    fbreg_loc = whereBytes - st.second[st.second.size() - 1].second.second - 16;
  }
  if (s->type->kind == ND_ARRAY_TYPE) {
    ArrayType *at = static_cast<ArrayType *>(s->type);
    // If the underlying type was AGAIN a struct
    if (structByteSizes.find(getUnderlying(at->underlying)) !=
        structByteSizes.end()) {
      fbreg_loc =
          -((variableCount -
             getByteSizeOfType(structByteSizes[getUnderlying(at->underlying)]
                                   .second.back()
                                   .second.first)) +
            16);
    }
  }
  pushLinker(
      ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
          "\n.long .L" + s->name +
          "_string\n"
          ".byte " +
          std::to_string(s->file_id) +
          "\n" // File index
          ".byte " +
          std::to_string(s->line) +
          "\n   " // Line number
          ".byte " +
          std::to_string(s->pos) +
          "\n" // Line column
          ".long .L" +
          asmName +
          "_debug_type\n" // Type - point to the DIE of the DW_TAG_base_type
          ".uleb128 " +
          std::to_string(                 // Length of data in location
              1 + sizeOfLEB(fbreg_loc)) + // DW_OP_fbreg (length of data)
          "\n.byte 0x91\n" // DW_OP_fbreg (first byte determines variable is
                           // offset from frame pointer rbp)
          ".sleb128 " +
          std::to_string(fbreg_loc) + "\n", // DW_OP_fbreg (actual offset)
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
                   "\n.quad .Ldie" + std::to_string(thisDieCount) +
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
    dwarf::useType(new SymbolType("long", SymbolType::Signedness::UNSIGNED));
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::EnumType) +
                   "\n.long .L" + s->name +
                   "_string"
                   "\n.byte " +
                   std::to_string(s->file_id) +           // File ID
                   "\n.byte " + std::to_string(s->line) + // Line number
                   "\n.byte " + std::to_string(s->pos) +  // Line column
                   "\n.byte 4" // each enum member is 4 bytes
                   "\n.byte 7" // encoding - DW_ATE_signed is 7
                   "\n.long .Llong_u_debug_type\n" // And yes, for some reason,
                                                   // this is still required.
               ,
               Section::DIE);
    dwarf::useStringP(s->name);
  }
  int fieldCount = 0;
  enumTable.emplace(s->name);
  for (IdentExpr *field : s->fields) {
    // Turn the enum field into an assembler constant
    push(Instr{.var = LinkerDirective{.value = ".set enum_" + s->name + "_" +
                                               field->name + ", " +
                                               std::to_string(fieldCount++) +
                                               "\n"},
               .type = InstrType::Linker},
         Section::ReadonlyData);

    // Add the enum field to the global table
    variableTable.insert({field->name, field->name});

    if (debug) {
      // Push the enum member DIE
      pushLinker(
          ".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::EnumMember) +
              "\n.long .L" + field->name +
              "_string"
              "\n.byte " +
              std::to_string(fieldCount - 1) + // Value of the enum member
              "\n",
          Section::DIE);
      // Push the name of the enum member
      dwarf::useStringP(field->name);
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
  long size = 0;
  std::vector<StructMember> members = {};
  for (std::pair<IdentExpr *, Node::Type *> field : s->fields) {
    long fieldSize = getByteSizeOfType(field.second);
    size += fieldSize; // Yes, even calculates the size of nested structs.
  }
  long subSize = 0;
  // Calculate size by adding the size of members
  for (long i = s->fields.size() - 1; i >= 0; i--) {
  // for (size_t i = 0; i < s->fields.size(); i++) {
    // Turn into offsets
    // For example: { short, int } -> { 2, 0 }
    // For example: { int, short, int } -> { 6, 2, 0 }

    // In both cases, the last element is stored at 0
    members.push_back(
      {s->fields.at(i).first->name, {s->fields.at(i).second, subSize}});

    // add the size of the NEXTT field
    if (i > 0)
      subSize += getByteSizeOfType(s->fields.at(i - 1).second);
    // if (i + 1 < s->fields.size())
    //   subSize += getByteSizeOfType(s->fields.at(i + 1).second);
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
                   "\n.short " + std::to_string(size) +   // Size of struct
                   "\n",
               Section::DIE);
    // Push name
    dwarf::useStringP(s->name);

    long currentByte = 0;
    for (size_t i = 0; i < s->fields.size(); i++) {
      auto &field = s->fields.at(i);
      // Push member DIE
      dwarf::useType(field.second);
      pushLinker(".uleb128 " +
                     std::to_string((int)dwarf::DIEAbbrev::StructMember) +
                     "\n.long .L" + field.first->name +
                     "_string"
                     "\n.long .L" +
                     type_to_diename(field.second) +
                     "_debug_type"
                     //  "\n.byte " + std::to_string(sizeOfLEB(-currentByte)) +
                     "\n.sleb128 " +
                     std::to_string(currentByte) + // Offset in struct
                     "\n",
                 Section::DIE);
      // push name in string
      dwarf::useStringP(field.first->name);

      // Add the byte size of the field
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
      // Entry point (Main) function is an unsigned int, meaning that it is
      // acceptable to return as a int here
      moveRegister("%rdi", "$0", DataSize::Qword, DataSize::Qword);
      handleExitSyscall();
      return;
    } else {
      handleReturnCleanup(); // We don't care about rax! We're exiting. We
                             // already know that nothing is being returned
                             // therefore we know that this is ok.
      std::string msg = "WARNING: Returning from a non-void function without a "
                        "return value. This is undefined behavior.";
      handleError(returnStmt->line, returnStmt->pos, msg, "codegen");
      return;
    }
  }
  // Generate return value for the function
  if (isEntryPoint) {
    // Depending on the size of the return value, we have to use the right kind
    // of pop size
    visitExpr(returnStmt->expr);
    switch (getByteSizeOfType(returnStmt->expr->asmType)) {
    case 1:
      push(Instr{.var = PopInstr{.where = "%dil", .whereSize = DataSize::Byte},
                 .type = InstrType::Pop},
           Section::Main);
      break;
    case 2:
      push(Instr{.var = PopInstr{.where = "%di", .whereSize = DataSize::Word},
                 .type = InstrType::Pop},
           Section::Main);
      break;
    case 4:
      push(Instr{.var = PopInstr{.where = "%edi", .whereSize = DataSize::Dword},
                 .type = InstrType::Pop},
           Section::Main);
      break;
    case 8:
    default:
      push(Instr{.var = PopInstr{.where = "%rdi", .whereSize = DataSize::Qword},
                 .type = InstrType::Pop},
           Section::Main);
      break;
    }
    handleExitSyscall();
    return;
  }
  if (returnStmt->expr->asmType->kind == ND_POINTER_TYPE ||
      returnStmt->expr->asmType->kind == ND_FUNCTION_TYPE ||
      returnStmt->expr->asmType->kind == ND_FUNCTION_TYPE_PARAM) {
    visitExpr(returnStmt->expr);
    popToRegister("%rax"); // rax is fine in this case- it is ALWAYS 8 bytes
    handleReturnCleanup();
    return;
  }
  // Probably a symbol type!
  SymbolType *st = static_cast<SymbolType *>(returnStmt->expr->asmType);
  if (st->name == "float" || st->name == "double") {
    visitExpr(returnStmt->expr);
    push(Instr{.var = PopInstr{.where = "%xmm0", .whereSize = intDataToSizeFloat(getByteSizeOfType(st))},
               .type = InstrType::Pop},
         Section::Main);
    handleReturnCleanup();
  } else {
    long byteSize = getByteSizeOfType(returnStmt->expr->asmType);
    if (byteSize <= 8) {
      if (returnStmt->expr->kind == ND_STRUCT) {
        // a struct literal. we will put this into a temporary variable
        // then push the temporary variable.
        declareStructVariable(returnStmt->expr, st->name, "%rbp",
                              variableCount);
        push(Instr{.var=PushInstr{
                    .what = std::to_string(-(variableCount)) + "(%rbp)",
                    .whatSize = intDataToSize(byteSize),},
                    .type = InstrType::Push},
              Section::Main);
      } else {
        visitExpr(returnStmt->expr);
      }
      switch (byteSize) {
      case 1:
        push(Instr{.var = PopInstr{.where = "%al", .whereSize = DataSize::Byte},
                   .type = InstrType::Pop},
             Section::Main);
        break;
      case 2:
        push(Instr{.var = PopInstr{.where = "%ax", .whereSize = DataSize::Word},
                   .type = InstrType::Pop},
             Section::Main);
        break;
      case 4:
        push(Instr{.var =
                       PopInstr{.where = "%eax", .whereSize = DataSize::Dword},
                   .type = InstrType::Pop},
             Section::Main);
        break;
      case 8:
      default:
        push(Instr{.var =
                       PopInstr{.where = "%rax", .whereSize = DataSize::Qword},
                   .type = InstrType::Pop},
             Section::Main);
        break;
      }
      handleReturnCleanup();
      return;
    }
    // TODO: Return things LARGER than 8 bytes!
    // If a struct:
    bool isStruct = structByteSizes.contains(st->name);
    if (isStruct) {
      if (byteSize <= 16) {
        // Return the two halves into the second register
        // Are we returning a literal?
        if (returnStmt->expr->kind == ND_STRUCT) {
          // Declare the struct as a variable-- put it into a temporary place of memory
          // then return it
          declareStructVariable(returnStmt->expr, st->name, "%rbp", variableCount);
          // We already know that it is greater than 8 bytes large, so we will automatically
          // put its fields into %rax and %rdi (%RAX = bottom half)

          push(Instr{.var = MovInstr{.dest = "%rdi",
                                     .src = std::to_string(-(variableCount)) + "(%rbp)",
                                     .destSize = DataSize::Qword,
                                     .srcSize = DataSize::Qword},
                     .type = InstrType::Mov},
               Section::Main);
          variableCount += 8;
          push(Instr{.var = MovInstr{.dest = "%rax",
                                     .src = std::to_string(-variableCount) + "(%rbp)",
                                     .destSize = DataSize::Qword,
                                     .srcSize = DataSize::Qword},
                     .type = InstrType::Mov},
                Section::Main);
          return;
        }
        visitExpr(returnStmt->expr);
      }
    }
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
    dwarf::useAbbrev(dwarf::DIEAbbrev::LexicalBlock);
    dwarf::useType(assignee->asmType);
    push(Instr{.var=Label{.name=".Ldie_loop" + std::to_string(loopCount) + "_begin"}, .type=InstrType::Label},Section::Main);
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::LexicalBlock) +
               "\n.byte " + std::to_string(s->file_id) +
               "\n.byte " + std::to_string(dynamic_cast<BlockStmt *>(s->block)->line) +
               "\n.byte " + std::to_string(dynamic_cast<BlockStmt *>(s->block)->pos) +
               // Low pc (the address of the start of the block- thats RIGHT NOW!)
               "\n.quad .Ldie_loop" + std::to_string(loopCount) + "_begin"
               "\n.quad .Ldie_loop" + std::to_string(loopCount) + "_end-.Ldie_loop" + std::to_string(loopCount) + "_begin\n",
            Section::DIE);
    // we know the type is always int
    pushLinker(".uleb128 " + std::to_string((int)dwarf::DIEAbbrev::Variable) +
                   "\n.long .L" + assignee->name +
                   "_string\n"
                   "\n.byte " +
                   std::to_string(s->file_id) +           // File ID
                   "\n.byte " + std::to_string(s->line) + // Line number
                   "\n.byte " + std::to_string(s->pos) +  // Line column
                   "\n.long .L" + type_to_diename(assignee->asmType) +
                   "_debug_type\n" // Type - point to the DIE of
                                   // the DW_TAG_base_type
                   "\n.uleb128 " +
                   std::to_string(1 + sizeOfLEB(-variableCount -
                                                8)) + // 1 byte is gonna follow
                   "\n.byte 0x91\n" // DW_OP_fbreg (first byte)
                   "\n.sleb128 " +
                   std::to_string(-variableCount - 8) + "\n",
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
  if (debug) dwarf::nextBlockDIE = false;
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
  if (debug) {
    push(Instr{.var=Label{.name=".Ldie_loop" + std::to_string(loopCount) + "_end"},.type=InstrType::Label},Section::Main);
    pushLinker(".byte 0\n", Section::DIE); // Explain that the LexicalBlock is over!
  }

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
  // Check if is a stupid loop (like if a test would fail like when checking 0
  // == 0)
  if (s->condition->kind != ND_BOOL) {
    JumpCondition jc = processComparison(s->condition);
    push(Instr{.var = JumpInstr{.op = getOpposite(jc), .label = postLoop},
               .type = InstrType::Jmp},
         Section::Main);
  }

  // yummers, now just do the block
  if (debug) dwarf::nextBlockDIE = true;
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
                              .label =
                                  "loop_post" + std::to_string(loopCount - 1)},
             .type = InstrType::Jmp},
       Section::Main);

  // Break statements are only valid inside loops
  // if (loopDepth < 1) {
  //   std::cerr << "Error: Break statement outside of loop" << std::endl;
  //   exit(-1);
  // }
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
    dwarf::nextBlockDIE = true;
    visitStmt(s->defaultCase);
    dwarf::nextBlockDIE = false;
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

    push(Instr{.var = CmpInstr{.lhs = "%rax",
                               .rhs = prevPush.what,
                               .size = DataSize::Qword},
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
    dwarf::nextBlockDIE = true;
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
  dwarf::nextBlockDIE = false;

  // We're done! Simply append the finishing label and we will be good to go.
  push(Instr{.var = Label{.name = matchEndWhere}, .type = InstrType::Label},
       Section::Main);
  conditionalCount += s->cases.size();
};

void codegen::dereferenceStructPtr(Node::Expr *expr, std::string structName,
                                   std::string offsetRegister,
                                   size_t startOffset) {
  // have DerefStruct: StructName = Struct&;
  // see how large the struct is
  DereferenceExpr *deref = static_cast<DereferenceExpr *>(expr);
  int structSize = structByteSizes[structName].first;
  // If the struct is <= 16, then hooray! We can skip all the complicated BS and just copy it straight up with a mov.
  if (structSize <= 16) {
    long structSizeRemaining = structSize;
    long runningTotal = 0;
    visitExpr(deref->left); // Structs are pushed as pointers because screw you
    popToRegister("%rdi");
    std::string registerName = "%r13";
    while (structSizeRemaining > 0) {
      DataSize pushSize = DataSize::Qword;
      if (structSizeRemaining >= 8) {
        pushSize = DataSize::Qword;
        registerName = "%r13";
      } else if (structSizeRemaining >= 4) {
        pushSize = DataSize::Dword;
        registerName = "%r13d";
      } else if (structSizeRemaining >= 2) {
        pushSize = DataSize::Word;
        registerName = "%r13w";
      } else {
        pushSize = DataSize::Byte; 
        registerName = "%r13b";
      }
      // Copy a quad word
      structSizeRemaining -= dataSizeToInt(pushSize);
      push(Instr{.var = MovInstr{.dest = registerName,
                                  .src = std::to_string(runningTotal) + "(%rdi)",
                                  .destSize = DataSize::Qword,
                                  .srcSize = pushSize},
                  .type = InstrType::Mov},
            Section::Main);
      runningTotal += dataSizeToInt(pushSize);
      push(Instr{.var = MovInstr{.dest = std::to_string(-((long long)startOffset + (structSizeRemaining))) + "(" + offsetRegister + ")",
                                  .src = registerName,
                                  .destSize = DataSize::Qword,
                                  .srcSize = pushSize},
                  .type = InstrType::Mov},
            Section::Main);
      // Subtract from remaining size
    }
    return;
  }
  // It's a big boy struct! This means we have to do some actual copying from a pointer

  // DO THIS LATER! IF YOU HAVE A STRUCT THIS BIG, YOU PROBABLY HAVE A PROBLEM
}

long int codegen::dataSizeToInt(DataSize size) {
  switch (size) {
    case DataSize::Byte:
      return 1;
    case DataSize::Word:
      return 2;
    case DataSize::Dword:
    case DataSize::SS:
      return 4;
    case DataSize::Qword:
    case DataSize::SD:
      return 8;
    case DataSize::None:
    default:
      return 0;
  }
}

// Structname passed by the varStmt's "type" field
void codegen::declareStructVariable(Node::Expr *expr, std::string structName,
                                    std::string offsetRegister,
                                    size_t startOffset) {
  if (expr->kind == ND_DEREFERENCE) { // have x: struct = struct&;
    dereferenceStructPtr(expr, structName, offsetRegister, startOffset);
    return; // We do not want to continue!
  }
  StructExpr *s = static_cast<StructExpr *>(expr);
  // Order the fields of the variable struct by the actual order of the declarad one
  std::vector<std::pair<std::string, Node::Expr *>> orderedFields = {};
  std::unordered_map<std::string, Node::Expr *> fields = {};
  for (auto &field : s->values) {
    // We need to get the type of the field, so we can order it
    fields.emplace(field.first->name, field.second);
  }
  orderStructFields(fields, structName, &orderedFields);
  // Now we can actually get to the fun part, where we evaluate each member
  for (long i = (signed)orderedFields.size() - 1; i >= 0; i--) {
  // for (size_t i = 0; i < orderedFields.size(); i++) {
    std::pair<std::string, Node::Expr *> &field = orderedFields[i];
    if (field.second->kind == ND_STRUCT) {
      declareStructVariable(field.second, getUnderlying(field.second->asmType), offsetRegister, startOffset + structByteSizes[structName].second[i].second.second);
      continue;
    }

    if (field.second->kind == ND_ARRAY) {
      // This is an array, so we need to declare it as such
      ArrayExpr *arr = static_cast<ArrayExpr *>(field.second);
      declareArrayVariable(arr, arr->elements.size());
      continue;
    }

    long int fieldSize = getByteSizeOfType(field.second->asmType);
    // Let's ignore other types of fields for now and only deal with normal variables
    if (fieldSize > 8) {
      // We cannot handle this yet, so we will just skip it
      std::string msg = "ERROR: Cannot handle struct field of size > 8 bytes yet";
      handleError(s->line, s->pos, msg, "codegen");
      continue;  
    }

    // Finally! A normal field!
    visitExpr(field.second);
    long fieldOffset = -(startOffset + structByteSizes[structName].second[i].second.second);
    DataSize fieldSizeData = intDataToSize(fieldSize);
    push(Instr{.var=PopInstr{
      .where = std::to_string(fieldOffset) + "(" + offsetRegister + ")",
      .whereSize = fieldSizeData},
      .type = InstrType::Pop},
    Section::Main);
  }
}

void codegen::orderStructFields(std::unordered_map<std::string, Node::Expr *> &fieldsToOrder, std::string &structName,
                       std::vector<std::pair<std::string, Node::Expr *>> *orderedFields) {
  // For each field to order, we must loop over the struct declaration
  for (auto &declaredField : structByteSizes[structName].second) {
    for (auto &field : fieldsToOrder) {
      if (declaredField.first == field.first) {
        // We found the field in the declaration, so we can add it to the ordered
        // fields
        orderedFields->push_back({declaredField.first, field.second});
        break; // 'break' will break the innermost loop
      }
    }
  }
}

void codegen::declareArrayVariable(Node::Expr *expr, long long arrayLength) {
  if (expr->kind == ND_ARRAY_AUTO_FILL) {
    // This is an implicit shorthand version of setting an array to [0, 0, 0, 0,
    // ....]
    ArrayAutoFill *s = static_cast<ArrayAutoFill *>(expr);
    long long totalSize = arrayLength * getByteSizeOfType(s->fillType);
    if (totalSize <= 256) {
      // Small enough for manual labor hehe
      // Ensure that filling happens from the top
      long long remainingBytes = totalSize;
      while (remainingBytes > 0) {
        if (remainingBytes >= 8) {
          // Fill with a quad word
          push(Instr{.var = MovInstr{.dest = std::to_string(
                                                 -((long long)variableCount +
                                                   remainingBytes)) +
                                             "(%rbp)",
                                     .src = "$0",
                                     .destSize = DataSize::Qword,
                                     .srcSize = DataSize::Qword},
                     .type = InstrType::Mov},
               Section::Main);
          remainingBytes -= 8;
          continue;
        } else if (remainingBytes >= 4) {
          // Fill with a long word
          push(Instr{.var = MovInstr{.dest = std::to_string(
                                                 -((long long)variableCount +
                                                   remainingBytes)) +
                                             "(%rbp)",
                                     .src = "$0",
                                     .destSize = DataSize::Dword,
                                     .srcSize = DataSize::Dword},
                     .type = InstrType::Mov},
               Section::Main);
          remainingBytes -= 4;
          continue;
        } else if (remainingBytes >= 2) {
          // Fill with a word
          push(Instr{.var = MovInstr{.dest = std::to_string(
                                                 -((long long)variableCount +
                                                   remainingBytes)) +
                                             "(%rbp)",
                                     .src = "$0",
                                     .destSize = DataSize::Word,
                                     .srcSize = DataSize::Word},
                     .type = InstrType::Mov},
               Section::Main);
          remainingBytes -= 2;
          continue;
        } else {
          // Fill with a single byte
          push(Instr{.var = MovInstr{.dest = std::to_string(
                                                 -((long long)variableCount +
                                                   remainingBytes)) +
                                             "(%rbp)",
                                     .src = "$0",
                                     .destSize = DataSize::Byte,
                                     .srcSize = DataSize::Byte},
                     .type = InstrType::Mov},
               Section::Main);
          remainingBytes -= 1;
          continue;
        }
      }
      return;
    }
    // C allocates 16 bytes near the top for some reason, let's rip them off and
    // do the same Assuming the autofill is small enough, we could manually fill
    // them with 0's mov by mov
    variableCount = (int64_t)round((size_t)variableCount, 16);
    push(Instr{.var = MovInstr{.dest = std::to_string(
                                           -(variableCount + totalSize)) +
                                       "(%rbp)",
                               .src = "$0",
                               .destSize = DataSize::Qword,
                               .srcSize = DataSize::Qword},
               .type = InstrType::Mov},
         Section::Main);
    push(Instr{.var = MovInstr{.dest = std::to_string(
                                           -(variableCount + totalSize - 8)) +
                                       "(%rbp)",
                               .src = "$0",
                               .destSize = DataSize::Qword,
                               .srcSize = DataSize::Qword},
               .type = InstrType::Mov},
         Section::Main);
    // Prepare the registers for the rep stosq instruction
    push(Instr{.var = LeaInstr{.size = DataSize::Qword,
                               .dest = "%rdx",
                               .src = "-" +
                                      std::to_string(variableCount +
                                                     arrayLength - 16) +
                                      "(%rbp)"},
               .type = InstrType::Lea},
         Section::Main);
    push(Instr{.var = XorInstr{.lhs = "%rax", .rhs = "%rax"},
               .type = InstrType::Xor},
         Section::Main);
    moveRegister("%rdi", "%rdx", DataSize::Qword, DataSize::Qword);
    long long newSize = totalSize - 16;
    if (newSize % 8 == 0) {
      moveRegister("%rcx", "$" + std::to_string(newSize / 8), DataSize::Qword,
                   DataSize::Qword);
      pushLinker("rep stosq\n\t",
                 Section::Main); // Repeat %rcx times to fill ptr %rdx with the
                                 // value of %rax (every 8 bytes)
    } else if (newSize % 4 == 0) {
      moveRegister("%rcx", "$" + std::to_string(newSize / 4), DataSize::Qword,
                   DataSize::Qword);
      pushLinker("rep stosd\n\t",
                 Section::Main); // Repeat %rcx times to fill ptr %rdx with the
                                 // value of %rax (every 8 bytes)
    } else if (newSize % 2 == 0) {
      moveRegister("%rcx", "$" + std::to_string(newSize / 2), DataSize::Qword,
                   DataSize::Qword);
      pushLinker("rep stosw\n\t",
                 Section::Main); // Repeat %rcx times to fill ptr %rdx with the
                                 // value of %rax (every 8 bytes)
    } else {
      moveRegister("%rcx", "$" + std::to_string(newSize), DataSize::Qword,
                   DataSize::Qword);
      pushLinker("rep stosb\n\t",
                 Section::Main); // Repeat %rcx times to fill ptr %rdx with the
                                 // value of %rax (every 8 bytes)
    }
    return;
  }
  ArrayExpr *s = static_cast<ArrayExpr *>(expr);
  ArrayType *at = static_cast<ArrayType *>(s->type);
  dwarf::useType(s->type);
  // Time for the fun part!! Woohoo!
  if (s->elements.size() == 0) return; // This means that it was defined as a placeholder. Let varDecl handle it.
  // Go through each element of the array one by one and pop it into place.
  // Also, arrays are backwards because fuck you
  long offsetTotal = s->elements.size() * getByteSizeOfType(at->underlying);
  for (long i = s->elements.size() - 1; i >= 0; i--) { // This condition is actually always true but shut up
    // What was the type of element?
    Node::Expr *element = s->elements[i];
    long sizeOfType = getByteSizeOfType(element->asmType);
    if (element->kind == ND_STRUCT) {
      offsetTotal -= sizeOfType;
      declareStructVariable(element, getUnderlying(element->asmType), "%rbp", variableCount + offsetTotal); // Maybe this will work, probably not though
      continue;
    }
    if (element->kind == ND_ARRAY) {
      offsetTotal -= sizeOfType;
      declareArrayVariable(element, dynamic_cast<ArrayType *>(element->asmType)->constSize);
      continue;
    }
    // Finally! The fun can begin.
    visitExpr(element);
    // Calculate where the hell we pop this
    DataSize size = intDataToSize(sizeOfType);
    offsetTotal -= sizeOfType;
    push(Instr{.var=PopInstr{.where = std::to_string(-(offsetTotal + variableCount)) + "(%rbp)",.whereSize=size},.type=InstrType::Pop},Section::Main);
  }
}