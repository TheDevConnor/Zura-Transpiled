#include "gen.hpp"
#include "optimize.hpp"
#include <sys/cdefs.h>

void codegen::visitStmt(Node::Stmt *stmt) {
  auto handler = lookup(stmtHandlers, stmt->kind);
  if (handler) {
    handler(stmt);
  }
}

void codegen::program(Node::Stmt *stmt) {
  auto s = static_cast<ProgramStmt *>(stmt);
  // I mean, its the program.
  // Not much to say about this one.
  for (Node::Stmt *stm : s->stmt) {
    if (stm->kind != ND_FN_STMT && stm->kind != ND_CONST_STMT) {
      // Shut up about the const statement
      std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
      std::cerr << "Global scope variables / statements not implemented! (StmtType: NodeKind[" << (int)stm->kind << "])" << std::endl;
      exit(-1);
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

  auto funcName = (s->name == "main") ? "usr_main" : "usr_" + s->name;
  push(Instr {.var=Label{.name=funcName},.type=InstrType::Label},Section::Main);

  push(Instr{.var = LinkerDirective{.value = ".cfi_startproc\n\t"},.type = InstrType::Linker},Section::Main);

  // Function args
  for (auto &args : s->params) {
    stackTable[args.first] = stackSize;
  }

  codegen::visitStmt(s->block);
  stackSize = preStackSize;

  push(Instr{.var = LinkerDirective{.value = ".cfi_endproc\n"},.type = InstrType::Linker},Section::Main);
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" + funcName + "\n\t"},.type = InstrType::Linker},Section::Main);
};

void codegen::varDecl(Node::Stmt *stmt) {
  auto s = static_cast<VarStmt *>(stmt);
  
  push(Instr {.var=Comment{.comment="Variable declaration for '" + s->name + "'"},.type=InstrType::Comment},Section::Main);

  visitExpr(static_cast<ExprStmt *>(s->expr)->expr);

  // add the variable to the symbol table
  stackTable[s->name] = stackSize;
  stackSize++;

  push(Instr {.var=Comment{.comment="End of variable declaration for '" + s->name + "'"},.type=InstrType::Comment},Section::Main);
};

void codegen::block(Node::Stmt *stmt) {
  auto s = static_cast<BlockStmt *>(stmt);
  // TODO: Track the number of variables and pop them off later
  // This should be handled by the IR when i get around to it though

  for (Node::Stmt *stm : s->stmts) {
    if (stm->kind == ND_FN_STMT) {
      // No 🫴
      std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
      std::cerr << "Nested functions not allowed!" << std::endl;
      exit(-1);
    }
    codegen::visitStmt(stm);
  }
};

void codegen::ifStmt(Node::Stmt *stmt) {
  auto s = static_cast<IfStmt *>(stmt);
  std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
  std::cerr << "If statements now implemented! (Condition: NodeKind[" << (int)s->condition->kind << "])" << std::endl;
  exit(-1);
};

void codegen::print(Node::Stmt *stmt) {
  auto s = static_cast<PrintStmt *>(stmt);
  std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
  std::cerr << "Print statements now implemented!" << std::endl;
  exit(-1);
};

void codegen::expr(Node::Stmt *stmt) {
  auto s = static_cast<ExprStmt *>(stmt);
  // Just evaluate the expression
  // TODO: Remove the last push statement or something idk its late
  codegen::visitExpr(s->expr);
};

void codegen::_return(Node::Stmt *stmt) {
  auto s = static_cast<ReturnStmt *>(stmt);
  
  codegen::visitExpr(s->expr);
  push(Instr {.var=PopInstr{.where="%rax"},.type=InstrType::Pop},Section::Main);
  stackSize--;

  push(Instr{.var = Ret{}, .type = InstrType::Ret}, Section::Main);
};

void codegen::_break(Node::Stmt *stmt) {
  auto s = static_cast<BreakStmt *>(stmt);
  std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
  std::cerr << "Break statements not implemented!" << std::endl;
  exit(-1);
};

void codegen::_continue(Node::Stmt *stmt) {
  auto s = static_cast<ContinueStmt *>(stmt);
  std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
  std::cerr << "Continue statements not implemented!" << std::endl;
  exit(-1);
};

void codegen::forLoop(Node::Stmt *stmt) {
  auto s = static_cast<ForStmt *>(stmt);
  std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
  std::cerr << "For loops not implemented!" << std::endl;
  exit(-1);
};

void codegen::whileLoop(Node::Stmt *stmt) {
  auto s = static_cast<WhileStmt *>(stmt);
  std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
  std::cerr << "While loops not implemented!" << std::endl;
  exit(-1);
};