#include "gen.hpp"
#include "optimizer/instr.hpp"
#include <sys/cdefs.h>

void codegen::visitStmt(Node::Stmt *stmt) {
  auto handler = lookup(stmtHandlers, stmt->kind);
  if (handler) {
    handler(stmt);
  }
}

void codegen::expr(Node::Stmt *stmt) {
  auto s = static_cast<ExprStmt *>(stmt);
  // Just evaluate the expression
  codegen::visitExpr(s->expr);
  text_section.pop_back();
};

void codegen::program(Node::Stmt *stmt) {
  auto s = static_cast<ProgramStmt *>(stmt);
  for (Node::Stmt *stm : s->stmt) {
    // check for a global variable declaration
    if (stm->kind == ND_VAR_STMT) {
      auto var = static_cast<VarStmt *>(stm);
      if (var->expr) {
        codegen::visitStmt(var);
      }
    }
    codegen::visitStmt(stm);
  }
};

void codegen::constDecl(Node::Stmt *stmt) {
  auto s = static_cast<ConstStmt *>(stmt);
  // TODO: Make ConstDecl's function like constant (immutable) variables
  codegen::visitStmt(s->value);
};

void codegen::funcDecl(Node::Stmt *stmt) {
  auto s = static_cast<FnStmt *>(stmt);
  int preStackSize = stackSize;

  isEntryPoint = (s->name == "main") ? true : false;
  auto funcName = (isEntryPoint) ? "main" : "usr_" + s->name;


  // WOO YEAH BABY DEBUG TIME

  if (debug) {
    SymbolType *st = static_cast<SymbolType *>(s->returnType);
    std::string asmName = st->name;
    pushLinker(".uleb128 0x3\n"
              ".byte 0x4\n" // DW_AT_external
              ".long .L" + funcName + "_string\n"
              ".byte 0x0\n" // File index 0 - no other file includes allowed (yet)
              ".byte " + std::to_string(s->line) + "\n" // Line number
              ".long .L" + asmName + "_debug_type\n" // Return type (DW_AT_type)
              ".quad .L" + funcName + "_debug_start\n" // Low pc
              ".quad .L" + funcName + "_debug_end - .L" + funcName + "_debug_start\n"
              ".uleb128 0x1\n" // 1 byte is gonna follow
              ".byte 0x9c\n"
              // No call all calls ??
              // Look, I'm not dealing with siblings right now
              , Section::DIE);

    pushLinker(".L" + funcName + "_string: .string \"" + funcName + "\"\n", Section::DIEString);
  }

  push(Instr{.var = LinkerDirective{.value = "\n.type " + funcName + ", @function"},.type = InstrType::Linker},Section::Main);
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},Section::Main);
  pushLinker(".L" + funcName + "_debug_start:\n", Section::Main);
  // push linker directive for the debug info (the line number)
  pushDebug(s->line);
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
  stackSize++; // Increase for the push of rbp
  funcBlockStart = stackSize;

  // Function args
  for (int i = 0; i < s->params.size(); i++) {
    // TODO: REPLACE THIS IMMEDIATELY!
    variableTable.insert({s->params.at(i).first, argOrder.at(i)});
  }

  codegen::visitStmt(s->block);
  stackSize = preStackSize;
  funcBlockStart = -1;
  // Function ends with ret so we can't really push any other instructions.
  pushLinker(".L" + funcName + "_debug_end:\n", Section::Main);

  push(Instr{.var = LinkerDirective{.value = ".cfi_endproc\n"},.type = InstrType::Linker},Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" + funcName + "\n\t"},
              .type = InstrType::Linker},Section::Main);
};

void codegen::varDecl(Node::Stmt *stmt) {
  auto s = static_cast<VarStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "Variable declaration for '" + s->name + "'"},
              .type = InstrType::Comment},Section::Main);

  // push another .loc
  pushDebug(s->line);

  // (the i = 4 part)
  // Evaluate the initializer expression, if present
  if (s->expr) {
    visitExpr(static_cast<ExprStmt *>(s->expr)->expr);
  }

  int whereBytes = variableCount++ * -8;
  std::string where = std::to_string(whereBytes) + "(%rbp)";
  popToRegister(where);
  // Update the symbol table with the variable's position
  variableTable.insert({s->name, where});

  push(Instr{.var = Comment{.comment = "End of variable declaration for '" + s->name + "'"},
              .type = InstrType::Comment},Section::Main);

  // Push DWARF DIE for variable declaration!!!!!
  if (!debug) return;
  std::string dieLabel = ".Ldie" + std::to_string(dieCount);
  SymbolType *st = static_cast<SymbolType *>(s->expr->expr->asmType);
  std::string asmName = st->name;

  pushLinker(".uleb128 0x2\n"
            ".long " + dieLabel + "_string\n"
            ".byte 0x0\n" // File index 0 - no other file includes allowed (yet)
            ".byte " + std::to_string(s->line) + "\n" // Line number
            ".long .L" + asmName + "_debug_type\n"
            ".uleb128 0x02\n" // Length of data in location definition
            ".byte 0x91\n" // DW_OP_fbreg
            ".sleb128 " + std::to_string(whereBytes) + "\n" // Offset of bytes - the 'x' from above
  , Section::DIE);

  // DIE String pointer
  push(Instr{.var = Label{.name = dieLabel + "_string"}, .type = InstrType::Label}, Section::DIEString);
  pushLinker(".string \"" + s->name + "\"\n", Section::DIEString);
  dieCount++;
}

void codegen::block(Node::Stmt *stmt) {
  auto s = static_cast<BlockStmt *>(stmt);
  // TODO: Track the number of variables and pop them off later
  // This should be handled by the IR when i get around to it though
  auto preSS = stackSize;
  auto preVS = variableCount;
  scopes.push_back(std::pair(preSS, preVS));
  // push a .loc
  pushDebug(s->line);
  for (Node::Stmt *stm : s->stmts) {
    codegen::visitStmt(stm);
  }
  scopes.pop_back();
  stackSize = preSS;
  variableCount = preVS;
};

void codegen::ifStmt(Node::Stmt *stmt) {
  auto s = static_cast<IfStmt *>(stmt);
  std::string preconCount = std::to_string(conditionalCount++);

  push(Instr{.var = Comment{.comment = "if statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line);

  BinaryExpr *cond = nullptr;
  if (s->condition->kind == ND_GROUP) {
    auto group = static_cast<GroupExpr *>(s->condition);
    cond = (group->expr->kind == ND_BINARY) ? static_cast<BinaryExpr *>(group->expr) : nullptr;
  } else {
    cond = static_cast<BinaryExpr *>(s->condition);
  }

  if (!cond) {
    // Handle non-binary conditions
    visitExpr(s->condition);
    popToRegister("%ecx");
    push(Instr{.var = CmpInstr{.lhs = "%ecx", .rhs = "$0"}, .type = InstrType::Cmp}, Section::Main);
    push(Instr{.var = JumpInstr{.op = JumpCondition::NotZero, .label = "conditional" + preconCount}, .type = InstrType::Jmp}, Section::Main);
  } else {
    // Process binary expression
    processBinaryExpression(cond, preconCount, "conditional");
  }

  // Jump to "main" label if condition is false (fall through)
  std::string elseLabel = "else" + preconCount;
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = elseLabel}, .type = InstrType::Jmp}, Section::Main);

  // True condition block: label and code for 'thenStmt'
  push(Instr{.var = Label{.name = "conditional" + preconCount}, .type = InstrType::Label}, Section::Main);
  visitStmt(s->thenStmt);

  // After executing 'thenStmt', jump to the end to avoid executing 'elseStmt'
  std::string endLabel = "main" + preconCount;
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
  auto s = static_cast<EnumStmt *>(stmt);

  // The feilds are pushed to the .data section
  int fieldCount = 0;
  for (auto &field : s->fields) {
    push(Instr{.var = Label{.name = field}, .type = InstrType::Label}, Section::ReadonlyData);
    push(Instr{.var = LinkerDirective{.value = ".long " + std::to_string(fieldCount++)}, .type = InstrType::Linker}, Section::ReadonlyData);

    // Add the enum field to the global table
    variableTable.insert({field, field});
  }

  // Add the enum to the global table
  variableTable.insert({s->name, ""}); // You should never refer to the enum base itself. You can only ever get its values
}

void codegen::structDecl(Node::Stmt *stmt) {
  auto s = static_cast<StructStmt *>(stmt);

  std::cerr << "Structs are not implemented yet!" << std::endl;
  exit(-1);
}


void codegen::print(Node::Stmt *stmt) {
  auto print = static_cast<PrintStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}}, Section::Main);
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;
  pushDebug(print->line);

  for (auto &arg : print->args) {
    std::string argType = static_cast<SymbolType *>(arg->asmType)->name;
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
      push(Instr{.var = Syscall({.name = "SYS_WRITE"}), .type = InstrType::Syscall}, Section::Main);
      break;
    } else if (argType == "int") {
      nativeFunctionsUsed[NativeASMFunc::itoa] = true;
      visitExpr(arg);
      popToRegister("%rax");
      push(Instr{.var = CallInstr{.name = "native_itoa"}}, Section::Main); // Convert int to string
      
      // Now we have the integer string in %rax (assuming %rax holds the pointer to the result)
      moveRegister("%rsi", "%rax", DataSize::Qword, DataSize::Qword);
      push(Instr{.var = CallInstr{.name = "native_strlen"}}, Section::Main);
      moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of number string
      
      // syscall id for write on x86 is 1
      moveRegister("%rax", "$1", DataSize::Qword, DataSize::Qword);
      // set rdi to 1 (file descriptor for stdout)
      moveRegister("%rdi", "$1", DataSize::Qword, DataSize::Qword);

      // Make syscall to write
      push(Instr{.var = Syscall({.name = "SYS_WRITE"}), .type = InstrType::Syscall}, Section::Main);
      break;
    } else {
      // Assume char* (string) type
      // If not, then ITS NOT OUR FAULT IT SEGFAULTS! YOUR code was trash!
      visitExpr(arg);
      popToRegister("%rsi");

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
  auto returnStmt = static_cast<ReturnStmt *>(stmt);

  pushDebug(returnStmt->line);
  // Generate return value for the function
  codegen::visitExpr(returnStmt->expr);

  // Store the return value in the RAX register
  popToRegister("%rax");

   if (isEntryPoint) {
     handleExitSyscall();
   } else {
     handleReturnCleanup();
     push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
   }
}

void codegen::forLoop(Node::Stmt *stmt) {
  auto s = static_cast<ForStmt *>(stmt);

  std::string preconCount = std::to_string(conditionalCount++);

  push(Instr{.var = Comment{.comment = "for loop"}, .type = InstrType::Comment}, Section::Main);

  // Create unique labels for the loop start and end
  std::string preLoopLabel = "loop_pre" + std::to_string(loopCount);
  std::string postLoopLabel = "loop_post" + std::to_string(loopCount++);
  
  // Declare the loop variable
  auto assign = static_cast<AssignmentExpr *>(s->forLoop);
  auto assignee = static_cast<IdentExpr *>(assign->assignee);

  push(Instr{.var = Comment{.comment = "For loop variable declaration"}, .type = InstrType::Comment}, Section::Main);

  variableTable.insert({assignee->name, std::to_string(-8 * variableCount++) + "(%rbp)"}); // Track the variable in the stack table
  pushDebug(s->line);
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
};

void codegen::whileLoop(Node::Stmt *stmt) {
  auto s = static_cast<WhileStmt *>(stmt);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "While loops not implemented!" << std::endl;
  exit(-1);
};

void codegen::_break(Node::Stmt *stmt) {
  auto s = static_cast<BreakStmt *>(stmt);
  
  push(Instr{.var = Comment{.comment = "break statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line);

  // Jump to the end of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = "loop_post" + std::to_string(loopCount - 1)}, .type = InstrType::Jmp}, Section::Main);

  // Break statements are only valid inside loops
  if (loopCount == 0) {
    std::cerr << "Error: Break statement outside of loop" << std::endl;
    exit(-1);
  }
};

void codegen::_continue(Node::Stmt *stmt) {
  auto s = static_cast<ContinueStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "continue statement"}, .type = InstrType::Comment}, Section::Main);
  pushDebug(s->line);

  // Jump back to the start of the loop
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned, .label = "loop_pre" + std::to_string(loopCount - 1)}, .type = InstrType::Jmp}, Section::Main);

  // Continue statements are only valid inside loops
  if (loopCount == 0) {
    std::cerr << "Error: Continue statement outside of loop" << std::endl;
    exit(-1);
  }
};