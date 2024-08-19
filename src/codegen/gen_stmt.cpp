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

  if (funcDecl->name == "main") {
    isEntryPoint = true;
    push(Instr{.var = Label{.name = "_start"}, .type = InstrType::Label}, true);
  } else {
    push(Instr{.var = Label{.name = "user_" + funcDecl->name}, .type = InstrType::Label},
         true);
  }

  // Todo: Handle function arguments
  // push arguments to the stack
  for (auto &args : funcDecl->params) {
    stackTable.insert({args.first, stackSize});
    stackSize++;
  }
  // Todo: Handle Function return type

  visitStmt(funcDecl->block);
  stackSize++;
}

void codegen::varDecl(Node::Stmt *stmt) {
  auto varDecl = static_cast<VarStmt *>(stmt);

  push(Instr{.var =
                 Comment{.comment = "define variable '" + varDecl->name + "'"},
             .type = InstrType::Comment},
       true);

  visitExpr(static_cast<ExprStmt *>(varDecl->expr)->expr);

  // add variable to the stack
  stackTable.insert({varDecl->name, stackSize});
}

void codegen::block(Node::Stmt *stmt) {
  auto block = static_cast<BlockStmt *>(stmt);
  for (auto &s : block->stmts) {
    visitStmt(s);
  }
}

void codegen::print(Node::Stmt *stmt) {
  auto print = static_cast<PrintStmt *>(stmt);

  push(Instr{.var = Comment{.comment = "print stmt"}}, true);

  for (auto &arg : print->args) {
    visitExpr(arg);

    push(Instr{.var = PopInstr({.where = "rsi"}), .type = InstrType::Pop},
         true);
    stackSize--;

    // set rdi to 1
    push(Instr{.var = MovInstr({.dest = "rdi", .src = "1"}),
               .type = InstrType::Mov},
         true);

    // set rdx to the length of the string
    auto str = static_cast<StringExpr *>(arg);
    push(Instr{.var = MovInstr(
                   {.dest = "rdx", .src = std::to_string(str->value.size())}),
               .type = InstrType::Mov},
         true);

    // syscall to write
    push(Instr{.var = MovInstr({.dest = "rax", .src = "1"}),
               .type = InstrType::Mov},
         true);
    push(Instr{.var = Syscall({.name = "SYS_WRITE"}),
               .type = InstrType::Syscall},
         true);
  }
}

void codegen::ifStmt(Node::Stmt *stmt) {
  auto ifstmt = static_cast<IfStmt *>(stmt);
  push(Instr{.var = Comment{.comment = "if statment"},
             .type = InstrType::Comment},
       true);

  std::string preConditionalCount = std::to_string(++conditionalCount);

  // visit the expr, jump if not zero
  visitExpr(ifstmt->condition);

  // pop value somewhere relatively unused, that is unlikely to be overriden
  // somewhere else
  push(Instr{.var = PopInstr{.where = "rcx"}, .type = InstrType::Pop}, true);
  push(Instr{.var = CmpInstr{.lhs = "rcx", .rhs = "0"}, .type = InstrType::Cmp},
       true);
  push(Instr{.var = JumpInstr{.op = JumpCondition::NotEqual,
                              .label = ("conditional" + preConditionalCount)},
             .type = InstrType::Jmp},
       true);

  if (ifstmt->elseStmt != nullptr) {
    visitStmt(ifstmt->elseStmt);
  }
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       true);

  push(Instr{.var = Label{.name = "conditional" + preConditionalCount},
             .type = InstrType::Label},
       true);
  visitStmt(ifstmt->thenStmt);
  push(Instr{.var = JumpInstr{.op = JumpCondition::Unconditioned,
                              .label = "main" + preConditionalCount},
             .type = InstrType::Jmp},
       true);

  push(Instr{.var = Label{.name = "main" + preConditionalCount},
             .type = InstrType::Label},
       true);
}

void codegen::retrun(Node::Stmt *stmt) {
  auto returnStmt = static_cast<ReturnStmt *>(stmt);

  if (isEntryPoint) {
    // check if we have an if return
    if (returnStmt->stmt != nullptr) {
      visitStmt(returnStmt->stmt);

      // pop the expression we just visited
      push(Instr{.var = PopInstr{.where = "rdi"}, .type = InstrType::Pop},
           true);
      stackSize--;

      push(Instr{.var = MovInstr{.dest = "rax", .src = "60"},
                 .type = InstrType::Mov},
           true);
      push(
          Instr{.var = Syscall{.name = "SYS_EXIT"}, .type = InstrType::Syscall},
          true);
      return;
    }

    visitExpr(returnStmt->expr);

    // pop the expression we just visited
    push(Instr{.var = PopInstr{.where = "rdi"}, .type = InstrType::Pop}, true);
    stackSize--;

    push(Instr{.var = MovInstr{.dest = "rax", .src = "60"},
               .type = InstrType::Mov},
         true);
    push(Instr{.var = Syscall{.name = "SYS_EXIT"}, .type = InstrType::Syscall},
         true);
    return;
  }

  // check if we have an if return
  if (returnStmt->stmt != nullptr) {
    visitStmt(returnStmt->stmt);

    // pop the expression we just visited
    push(Instr{.var = PopInstr{.where = "rdi"}, .type = InstrType::Pop}, true);
    stackSize--;
    push(Instr{.var = Ret{}, .type = InstrType::Ret}, true);
    return;
  }

  visitExpr(returnStmt->expr);
  push(Instr{.var = PopInstr{.where = "rdi"}, .type = InstrType::Pop}, true);
  stackSize--;
  push(Instr{.var = Ret{}, .type = InstrType::Ret}, true);
}