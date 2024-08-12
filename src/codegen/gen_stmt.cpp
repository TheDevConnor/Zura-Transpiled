#include "gen.hpp"

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
    push("_start:", true);
  } else {
    push("_" + funcDecl->name + ":", true);
  }

  // Todo: Handle function arguments
  // Todo: Handle Function return type

  visitStmt(funcDecl->block);
  stackSize++;
}

void codegen::varDecl(Node::Stmt *stmt) {
  auto varDecl = static_cast<VarStmt *>(stmt);

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

    // check if the return value is a single number on the node
    if (returnStmt->expr->kind == ND_NUMBER) {
      push("\tpop rdi", true);
      stackSize--;
    }

    if (returnStmt->expr->kind == ND_IDENT) {
      push("\tpop rdi", true);
      stackSize--;
    }

    push("\tmov rax, 60", true);
    push("\tsyscall", true);
    return;
  }
  visitExpr(returnStmt->expr);
  push("\tpop rdi", true);
  stackSize--;
  push("\tret", true);
}
