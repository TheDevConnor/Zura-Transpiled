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
  auto expr = static_cast<ExprStmt *>(stmt);
  visitExpr(expr->expr);
}

void codegen::program(Node::Stmt *stmt) {
  auto program = static_cast<ProgramStmt *>(stmt);

  for (auto &s : program->stmt) {
    visitStmt(s);
  }
}

void codegen::constDecl(Node::Stmt *stmt) {
  auto constDecl = static_cast<ConstStmt *>(stmt);
  visitStmt(constDecl->value);
}

void codegen::funcDecl(Node::Stmt *stmt) {
  auto funcDecl = static_cast<fnStmt *>(stmt);
  // Declare the type of this function (important)

  std::string funcName = "usr_" + funcDecl->name;
  int preStackSize = stackSize;
  push(Instr{.var = LinkerDirective{.value =
                                        ".type " + funcName + ", @function\n"},
             .type = InstrType::Linker},
       Section::Main);
  push(Instr{.var = Label{.name = funcName}, .type = InstrType::Label},
       Section::Main);
  // Pre-body function stuff
  push(Instr{.var = LinkerDirective{.value = ".cfi_startproc\n\t"},
             .type = InstrType::Linker},
       Section::Main);
  push(Instr{.var = PushInstr{.what = "%rbp", .whatSize = DataSize::Qword},
             .type = InstrType::Mov},
       Section::Main);
  stackSize++;
  push(Instr{.var = MovInstr{.dest = "%rbp",
                             .src = "%rsp",
                             .destSize = DataSize::Qword,
                             .srcSize = DataSize::Qword},
             .type = InstrType::Mov},
       Section::Main);
  // push arguments to the stack
  for (auto &args : funcDecl->params) {
    stackTable.insert({args.first, stackSize});
  }

  visitStmt(funcDecl->block);
  stackSize = preStackSize;

  // for (auto &arg : funcDecl->params) {
  // }

  // Yes, AFTER the ret! and in the main function as well!
  push(Instr{.var = LinkerDirective{.value = ".cfi_endproc\n\t"},
             .type = InstrType::Linker},
       Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" +
                                             funcName + "\n\t"},
             .type = InstrType::Linker},
       Section::Main);
  // return in rax
}

void codegen::varDecl(Node::Stmt *stmt) {
  auto varDecl = static_cast<VarStmt *>(stmt);

  push(Instr{.var =
                 Comment{.comment = "define variable '" + varDecl->name + "'"},
             .type = InstrType::Comment},
       Section::Main);

  visitExpr(static_cast<ExprStmt *>(varDecl->expr)->expr);

  // add variable to the stack
  stackTable.insert({varDecl->name, stackSize});
}

void codegen::block(Node::Stmt *stmt) {
  auto block = static_cast<BlockStmt *>(stmt);
  stackSizesForScopes.push_back(stackSize);
  size_t scopeCount = stackSizesForScopes.size();
  for (auto &s : block->stmts) {
    visitStmt(s);
  }
  // If difference is zero, it will skip!
  if (howBadIsRbp > 0)
    push(Instr{.var=AddInstr{.lhs="%rbp",.rhs="$"+std::to_string(howBadIsRbp)},.type=InstrType::Add},Section::Main);
  if (stackSizesForScopes.size() < scopeCount)
    return; // A function must have returned, meaning rsp and stuff will already
            // be handled
  if (int diff =
          stackSize - stackSizesForScopes.at(stackSizesForScopes.size() - 1)) {
    push(Instr{.var = AddInstr{.lhs = "%rsp",
                               .rhs = "$" + std::to_string(8 * diff)},
               .type = InstrType::Add},
         Section::Main);

    stackSize = stackSizesForScopes.at(stackSizesForScopes.size() - 1);
    stackSizesForScopes.pop_back(); // we dont need it anymore
  }
}

void codegen::print(Node::Stmt *stmt) {
  auto print = static_cast<PrintStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}}, Section::Main);
  nativeFunctionsUsed[NativeASMFunc::strlen] = true;

  for (auto &arg : print->args) {
    // visitExpr(arg);
    switch (arg->kind) {
    case NodeKind::ND_NUMBER: {
      auto number = static_cast<NumberExpr *>(arg);
      std::string label = "num" + std::to_string(stringCount++);

      // Push the label onto the stack
      push(Instr{.var = PushInstr{.what = '$' + label,
                                  .whatSize = DataSize::Qword},
                 .type = InstrType::Push},
           Section::Main);
      stackSize++;

      // Define the label for the string
      push(Instr{.var = Label{.name = label}, .type = InstrType::Label},
           Section::Data);

      // Store the processed string
      push(Instr{.var =
                     AscizInstr{.what = "\"" +
                                        std::to_string(size_t(number->value)) +
                                        "\""},
                 .type = InstrType::Asciz},
           Section::Data);

      break;
    }
    default:
      visitExpr(arg);
      break;
    }
    // assume type-checker worked properly and a string is passed in
    push(Instr{.var = PopInstr({.where = "%rsi", .whereSize = DataSize::Qword}),
               .type = InstrType::Pop},
         Section::Main);
    stackSize--;

    // calculate string length using native function
    push(Instr{.var = MovInstr{.dest = "%rdi",
                               .src = "%rsi",
                               .destSize = DataSize::Qword,
                               .srcSize = DataSize::Qword},
               .type = InstrType::Mov},
         Section::Main);
    push(Instr{.var = CallInstr{.name = "native_strlen"}}, Section::Main);

    auto str = static_cast<StringExpr *>(arg);
    push(Instr{.var = MovInstr({.dest = "%rdx",
                                .src = "%rax",
                                .destSize = DataSize::Qword,
                                .srcSize = DataSize::Qword}),
               .type = InstrType::Mov},
         Section::Main);

    // syscall id for write on x86 is 1
    push(Instr{.var = MovInstr({.dest = "%rax",
                                .src = "$1",
                                .destSize = DataSize::Qword,
                                .srcSize = DataSize::Qword}),
               .type = InstrType::Mov},
         Section::Main);

    // set rdi to 1 (file descriptor for stdout)
    push(Instr{.var = MovInstr({.dest = "%rdi",
                                .src = "$1",
                                .destSize = DataSize::Qword,
                                .srcSize = DataSize::Qword}),
               .type = InstrType::Mov},
         Section::Main);

    // create call
    push(Instr{.var = Syscall({.name = "SYS_WRITE"}),
               .type = InstrType::Syscall},
         Section::Main);
  }
}

void codegen::ifStmt(Node::Stmt *stmt) {
  auto ifstmt = static_cast<IfStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "if statment"},
             .type = InstrType::Comment},
       Section::Main);

  std::string preConditionalCount = std::to_string(++conditionalCount);

  // visit the expr, jump if not zero
  visitExpr(ifstmt->condition);

  // pop value somewhere relatively unused, that is unlikely to be overriden
  // somewhere else
  push(Instr{.var = PopInstr{.where = "%rcx"}, .type = InstrType::Pop},
       Section::Main);
  stackSize--;
  // NOTE: This is not a directive! Test Instr does not exist, so pushing
  // plaintext is easier
  push(Instr{.var = LinkerDirective{.value = "test %rcx, %rcx\n\t"},
             .type = InstrType::Cmp},
       Section::Main);
  push(Instr{.var = JumpInstr{.op = JumpCondition::NotZero,
                              .label = ("conditional" + preConditionalCount)},
             .type = InstrType::Jmp},
       Section::Main);
  // Evaluate else
  if (ifstmt->elseStmt != nullptr) {
    visitStmt(ifstmt->elseStmt);
  }
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       Section::Main);

  push(Instr{.var = Label{.name = "conditional" + preConditionalCount},
             .type = InstrType::Label},
       Section::Main);
  visitStmt(ifstmt->thenStmt);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       Section::Main);

  push(Instr{.var = Label{.name = "main" + preConditionalCount},
             .type = InstrType::Label},
       Section::Main);
}

void codegen::_return(Node::Stmt *stmt) {
  auto returnStmt = static_cast<ReturnStmt *>(stmt);
  // the "main" function is handled as a user function- we dont need to make any
  // exceptions check if we have an if return
  if (returnStmt->stmt != nullptr) {
    visitStmt(returnStmt->stmt);

    // pop the expression we just visited
    push(Instr{.var = PopInstr{.where = "%rax", .whereSize = DataSize::Qword},
               .type = InstrType::Pop},
         Section::Main);
    stackSize--;

    if (howBadIsRbp > 0)
      push(Instr{.var=AddInstr{.lhs="%rbp",.rhs="$"+std::to_string(howBadIsRbp)},.type=InstrType::Add},Section::Main);
  
    push(Instr{.var = MovInstr{.dest = "%rsp",
                               .src = "%rbp",
                               .destSize = DataSize::Qword,
                               .srcSize = DataSize::Qword},
               .type = InstrType::Mov},
         Section::Main);
    push(Instr{.var = PopInstr{.where = "%rbp", .whereSize = DataSize::Qword},
               .type = InstrType::Pop},
         Section::Main);
    stackSize--;
    push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
    return;
  }

  visitExpr(returnStmt->expr);
  push(Instr{.var = PopInstr{.where = "%rax", .whereSize = DataSize::Qword},
             .type = InstrType::Pop},
       Section::Main);
  stackSize--;
  // let the stack and stuff handle that out
  // also, add back the value of "how bad is rbp"

  if (howBadIsRbp > 0)
    push(Instr{.var = AddInstr{.lhs = "%rbp",
                               .rhs = "$" + std::to_string(howBadIsRbp)},
               .type = InstrType::Add},
         Section::Main);

  push(Instr{.var = MovInstr{.dest = "%rsp",
                             .src = "%rbp",
                             .destSize = DataSize::Qword,
                             .srcSize = DataSize::Qword},
             .type = InstrType::Mov},
       Section::Main);
  push(Instr{.var = PopInstr{.where = "%rbp", .whereSize = DataSize::Qword},
             .type = InstrType::Pop},
       Section::Main);
  stackSize--;
  push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
}

void codegen::whileLoop(Node::Stmt *stmt) {
  auto whileLoop = static_cast<WhileStmt *>(stmt);

  std::string preLoopLabel = "pre_loop" + std::to_string(loopCount);
  std::string postLoopLabel = "post_loop" + std::to_string(loopCount++);

  push(Instr{.var = Comment{.comment = "while loop"},
             .type = InstrType::Comment},
       Section::Main);
  // evalute condition once before in main func
  push(Instr{.var = Label { .name = preLoopLabel }, .type = InstrType::Label}, Section::Main);
  visitExpr(whileLoop->condition);
  // result in 0x0 or 0x1 on stack
  push(Instr{.var = PopInstr{.where="%rcx", .whereSize = DataSize::Qword}, .type = InstrType::Pop}, Section::Main);
  stackSize--;
  // Result of condition in rcx
  push(Instr{.var = LinkerDirective{.value="test %rcx, %rcx\n\t"}, .type = InstrType::Linker}, Section::Main);
  // Condition failed already? That's too sad. Ignore the rest of the loop!
  push(Instr{.var = JumpInstr { .op = JumpCondition::Zero, .label = postLoopLabel }, .type = InstrType::Jmp }, Section::Main);
  
  // Condition passed, start loop body
  visitStmt(whileLoop->block);
  // Loop body is over, execute optional
  if (whileLoop->optional != nullptr) {
	visitExpr(whileLoop->optional);
  }
  push(Instr{.var = JumpInstr { .op = JumpCondition::Unconditioned, .label = preLoopLabel }, .type = InstrType::Jmp }, Section::Main);

  push(Instr{.var = Label { .name = postLoopLabel }, .type = InstrType::Label }, Section::Main);
  conditionalCount++;
}

void codegen::forLoop(Node::Stmt *stmt) {
  auto forLoop = static_cast<ForStmt *>(stmt);

  std::string preLoopLabel = "pre_loop" + std::to_string(loopCount);
  std::string postLoopLabel = "post_loop" + std::to_string(loopCount++);

  push(Instr{.var = Comment{.comment = "for loop"},
			 .type = InstrType::Comment},
	   Section::Main);
  
  // declare the variable
  auto assign = static_cast<AssignmentExpr *>(forLoop->forLoop);
  auto assignee = static_cast<IdentExpr *>(assign->assignee);
  push(Instr{.var = Comment{.comment = "define variable '" + assignee->name + "'"},
			 .type = InstrType::Comment},
	   Section::Main);
  push(Instr{.var = PushInstr{.what = "$0x0"}, .type = InstrType::Push}, Section::Main);
  stackSize++;
  stackTable.insert({assignee->name, stackSize});
  visitExpr(assign);

  // evalute condition once before in main func
  push(Instr{.var = Label { .name = preLoopLabel }, .type = InstrType::Label}, Section::Main);
  visitExpr(forLoop->condition);
  // result in 0x0 or 0x1 on stack
  push(Instr{.var = PopInstr{.where="%rcx", .whereSize = DataSize::Qword}, .type = InstrType::Pop}, Section::Main);
  stackSize--;
  // Result of condition in rcx
  push(Instr{.var = LinkerDirective{.value="test %rcx, %rcx\n\t"}, .type = InstrType::Linker}, Section::Main);
  // Condition failed already? That's too sad. Ignore the rest of the loop!
  push(Instr{.var = JumpInstr { .op = JumpCondition::Zero, .label = postLoopLabel }, .type = InstrType::Jmp }, Section::Main);
  
  // Condition passed, start loop body
  visitStmt(forLoop->block);
  // Loop body is over, execute optional
  if (forLoop->optional != nullptr) { 
  	visitExpr(forLoop->optional);
  }
  push(Instr{.var = JumpInstr { .op = JumpCondition::Unconditioned, .label = preLoopLabel }, .type = InstrType::Jmp }, Section::Main);

  push(Instr{.var = Label { .name = postLoopLabel }, .type = InstrType::Label }, Section::Main);
  conditionalCount++;
}