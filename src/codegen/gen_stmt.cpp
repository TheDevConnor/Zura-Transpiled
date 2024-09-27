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

  auto funcName = (s->name == "main") ? s->name : "usr_" + s->name;
  push(Instr{.var = LinkerDirective{.value = "\n.type " + funcName + ", @function"},.type = InstrType::Linker},Section::Main);
  push(Instr {.var=Label{.name=funcName},.type=InstrType::Label},Section::Main);

  push(Instr{.var = LinkerDirective{.value = ".cfi_startproc\n\t"},.type = InstrType::Linker},Section::Main);

  push(Instr{.var=PushInstr{.what="%rbp",.whatSize=DataSize::Qword},.type=InstrType::Push},Section::Main);
  push(Instr{.var=MovInstr{.dest="%rbp",.src="%rsp",.destSize=DataSize::Qword,.srcSize=DataSize::Qword},.type=InstrType::Mov},Section::Main);
  if (s->name == "main") {
    stackSize = 0;
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
  push(Instr{.var = LinkerDirective{.value = ".size " + funcName + ", .-" + funcName + "\n\t"},.type = InstrType::Linker},Section::Main);
};

void codegen::varDecl(Node::Stmt *stmt) {
    auto s = static_cast<VarStmt *>(stmt);

    push(Instr {.var=Comment{.comment="Variable declaration for '" + s->name + "'"},.type=InstrType::Comment},Section::Main);

    // Evaluate the initializer expression, if present
    if (s->expr) {
        visitExpr(static_cast<ExprStmt *>(s->expr)->expr);
    }

    // Update the symbol table with the variable's position
    stackTable[s->name] = stackSize;

    push(Instr {.var=Comment{.comment="End of variable declaration for '" + s->name + "'"},.type=InstrType::Comment}, Section::Main);
}


void codegen::block(Node::Stmt *stmt) {
  auto s = static_cast<BlockStmt *>(stmt);
  // TODO: Track the number of variables and pop them off later
  // This should be handled by the IR when i get around to it though
  auto preSS = stackSize;
  scopes.push_back(preSS);
  for (Node::Stmt *stm : s->stmts) {
    if (stm->kind == ND_FN_STMT) {
      // No 🫴
      std::cerr << "No fancy error for this, beg Connor... (Soviet Pancakes speaking)" << std::endl;
      std::cerr << "Nested functions not allowed!" << std::endl;
      exit(-1);
    }
    codegen::visitStmt(stm);
  }
  scopes.pop_back();
  stackSize = preSS;
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
  // Pop the stacksize (important for no segfaults (: )
  if (stackSize - funcBlockStart == 0) {
    push(Instr {.var=PopInstr{.where="%rsp",.whereSize=DataSize::Qword},.type=InstrType::Pop},Section::Main);
    stackSize--;
  } else {
    push(Instr{.var=MovInstr{.dest="%rbp",.src=std::to_string(8 * (stackSize-funcBlockStart))+"(%rsp)",.destSize=DataSize::Qword,.srcSize=DataSize::Qword},.type=InstrType::Mov},Section::Main);
  }
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