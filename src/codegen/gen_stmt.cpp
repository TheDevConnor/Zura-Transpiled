#include "gen.hpp"
#include "optimize.hpp"
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
  // TODO: Remove the last push statement or something idk its late
  codegen::visitExpr(s->expr);
};

void codegen::program(Node::Stmt *stmt) {
  auto s = static_cast<ProgramStmt *>(stmt);
  for (Node::Stmt *stm : s->stmt) {
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

  push(Instr{.var = LinkerDirective{.value = "\n.type " + funcName + ", @function"},.type = InstrType::Linker},Section::Main);
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".cfi_startproc\n\t"},.type = InstrType::Linker},Section::Main);

  if (isEntryPoint) {
    stackSize = 0;
  } else {
    push(Instr{.var = PushInstr{.what = "%rbp", .whatSize = DataSize::Qword},.type = InstrType::Push},Section::Main);
    stackSize++;
    push(Instr{.var = MovInstr{.dest = "%rbp",
                                      .src = "%rsp",
                                      .destSize = DataSize::Qword,
                                      .srcSize = DataSize::Qword},
               .type = InstrType::Mov},Section::Main);
  }
  stackSize++; // Increase for the push of rbp
  funcBlockStart = stackSize;

  // Function args
  for (auto &args : s->params) {
    stackTable[args.first] = stackSize;
  }

  codegen::visitStmt(s->block);
  stackSize = preStackSize;
  funcBlockStart = -1;
  // Function ends with ret so we can't really push any other instructions.

  push(Instr{.var = LinkerDirective{.value = ".cfi_endproc\n"},.type = InstrType::Linker},Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" + funcName + "\n\t"},
              .type = InstrType::Linker},Section::Main);
};

void codegen::varDecl(Node::Stmt *stmt) {
  auto s = static_cast<VarStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "Variable declaration for '" + s->name + "'"},
              .type = InstrType::Comment},Section::Main);

  // Evaluate the initializer expression, if present
  if (s->expr) {
    visitExpr(static_cast<ExprStmt *>(s->expr)->expr);
  }

  // Update the symbol table with the variable's position
  stackTable.insert({s->name, stackSize});

  push(Instr{.var = Comment{.comment = "End of variable declaration for '" + s->name + "'"},
              .type = InstrType::Comment},Section::Main);
}

void codegen::block(Node::Stmt *stmt) {
  auto s = static_cast<BlockStmt *>(stmt);
  // TODO: Track the number of variables and pop them off later
  // This should be handled by the IR when i get around to it though
  auto preSS = stackSize;
  scopes.push_back(preSS);
  for (Node::Stmt *stm : s->stmts) {
    codegen::visitStmt(stm);
  }
  scopes.pop_back();
  stackSize = preSS;
};

void codegen::ifStmt(Node::Stmt *stmt) {
  auto s = static_cast<IfStmt *>(stmt);
  std::string preconCount = std::to_string(conditionalCount++);

  push(Instr{.var = Comment{.comment = "if statement"}, .type = InstrType::Comment}, Section::Main);

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
    popToRegister("%rcx");
    push(Instr{.var = CmpInstr{.lhs = "%rcx", .rhs = "$0"}, .type = InstrType::Cmp}, Section::Main);
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

void codegen::print(Node::Stmt *stmt) {
  auto print = static_cast<PrintStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}}, Section::Main);
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;

  for (auto &arg : print->args) {
    switch (arg->kind) {
    case ND_STRING: {
      visitExpr(arg);
      popToRegister("%rsi"); // String address
      moveRegister("%rdi", "%rsi", DataSize::Qword, DataSize::Qword);
      push(Instr{.var = CallInstr{.name = "native_strlen"}}, Section::Main);
      moveRegister("%rdx", "%rax", DataSize::Qword, DataSize::Qword); // Length of string

      // syscall id for write on x86 is 1
      moveRegister("%rax", "$1", DataSize::Qword, DataSize::Qword);
      // set rdi to 1 (file descriptor for stdout)
      moveRegister("%rdi", "$1", DataSize::Qword, DataSize::Qword);

      // Make syscall to write
      push(Instr{.var = Syscall({.name = "SYS_WRITE"}), .type = InstrType::Syscall}, Section::Main);
      break;
    }

    case ND_INT: {
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
    }

    default: {
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
}

void codegen::_return(Node::Stmt *stmt) {
  auto returnStmt = static_cast<ReturnStmt *>(stmt);

  // Generate return value for the function
  codegen::visitExpr(returnStmt->expr);

  // Store the return value in the RAX register
  push(Instr{.var = PopInstr{.where = "%rax"}, .type = InstrType::Pop}, Section::Main);
  stackSize--;

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

  stackTable.insert({assignee->name, stackSize}); // Track the variable in the stack table
  visitExpr(assign);  // Process the initial loop assignment (e.g., i = 0)

  // Set loop start label
  push(Instr{.var = Label{.name = preLoopLabel}, .type = InstrType::Label}, Section::Main);
  // Evaluate the loop condition
  processBinaryExpression(static_cast<BinaryExpr *>(s->condition), preconCount, "loop_post", true);

  // Execute the loop body (if condition is true)
  visitStmt(s->block);  // Visit the statements inside the loop body

  // Evaluate the loop increment (e.g., i++)
  if (s->optional != nullptr) {
    visitExpr(s->optional);  // Process the loop increment if provided
    text_section.pop_back();
  }

  // Jump back to the start of the loop
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
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Break statements not implemented!" << std::endl;
  exit(-1);
};

void codegen::_continue(Node::Stmt *stmt) {
  auto s = static_cast<ContinueStmt *>(stmt);
  std::cerr
      << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)"
      << std::endl;
  std::cerr << "Continue statements not implemented!" << std::endl;
  exit(-1);
};