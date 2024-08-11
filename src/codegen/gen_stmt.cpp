#include "gen.hpp"

void codegen::visitStmt(Node::Stmt *stmt) {
  auto handler = lookup(stmtHandlers, stmt->kind);
  if (handler) {
    handler(stmt);
  }
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
    push("global _start", true);
    push("_start:", true);
  } else {
    push("_" + funcDecl->name + ":", true);
  }

  // Todo: Handle function arguments
  // Todo: Handle Function return type

  visitStmt(funcDecl->block);
}

void codegen::varDecl(Node::Stmt *stmt) {
  // Handle variable declaration if necessary
}

void codegen::block(Node::Stmt *stmt) {
  auto block = static_cast<BlockStmt *>(stmt);
  for (auto &s : block->stmts) {
    visitStmt(s);
  }
}

void codegen::retrun(Node::Stmt *stmt) {
  auto returnStmt = static_cast<ReturnStmt *>(stmt);

  push("; return statement", true);
  push("mov rax, 60", true);

  visitExpr(returnStmt->expr);

  push("pop rdi", true);
  push("syscall", true);
}
