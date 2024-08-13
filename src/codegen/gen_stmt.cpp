#include "gen.hpp"
#include "optimize.hpp"

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
    push(Optimezer::Instr { .var = Label { .name = "_start" }, .type = InstrType::Label }, true);
  } else {
    push(Optimezer::Instr { .var = Label { .name = funcDecl->name }, .type = InstrType::Label }, true);
  }

  // Todo: Handle function arguments
  // Todo: Handle Function return type

  visitStmt(funcDecl->block);
  stackSize++;
}

void codegen::varDecl(Node::Stmt *stmt) {
  auto varDecl = static_cast<VarStmt *>(stmt);
  // it builds without the error?
  push(Optimezer::Instr { .var = Comment { .comment = "define variable '" + varDecl->name + "'" }, .type = InstrType::Comment }, true);
  visitStmt(varDecl->expr);

  // add variable to the stack
  stackTable.insert({varDecl->name, stackSize});
}

void codegen::block(Node::Stmt *stmt) {
  auto block = static_cast<BlockStmt *>(stmt);
  for (auto &s : block->stmts) {
    visitStmt(s);
  }
}

void codegen::retrun(Node::Stmt *stmt) {
  auto returnStmt = static_cast<ReturnStmt *>(stmt);

  if (isEntryPoint) {
    visitExpr(returnStmt->expr);

    // pop the expression we just visited
    push(Optimezer::Instr { .var = PopInstr { .where = "rdi" }, .type = InstrType::Pop }, true);
    stackSize--;
    
    push(Optimezer::Instr { .var = MovInstr { .dest = "rax", .src = "60" }, .type = InstrType::Mov }, true);
    push(Optimezer::Instr { .var = Syscall { .name = "SYS_EXIT" }, .type = InstrType::Syscall }, true);
    return;
  }
  visitExpr(returnStmt->expr);
  push(Optimezer::Instr { .var = PopInstr { .where = "rdi" }, .type = InstrType::Pop }, true);
  stackSize--;
  push(Optimezer::Instr { .var = Ret {}, .type = InstrType::Ret }, true);
}
