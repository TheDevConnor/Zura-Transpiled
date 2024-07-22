#pragma once

#include "ast.hpp"

#include <iostream>
#include <string>
#include <vector>

class ProgramStmt : public Node::Stmt {
public:
  std::vector<Node::Stmt *> stmt;

  ProgramStmt(std::vector<Node::Stmt *> stmt) : stmt(stmt) {
    kind = NodeKind::ND_PROGRAM;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ProgramStmt: \n";
    for (auto s : stmt) {
      s->debug(ident + 1);
    }
  }

  ~ProgramStmt() {
    for (auto s : stmt) {
      delete s;
    }
  }
};

class ExprStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *expr;

  ExprStmt(int line, int pos, Node::Expr *expr)
      : line(line), pos(pos), expr(expr) {
    kind = NodeKind::ND_EXPR_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ExprStmt: \n";
    expr->debug(ident + 1);
  }

  ~ExprStmt() { delete expr; }
};

class ConstStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  Node::Stmt *value;

  ConstStmt(int line, int pos, std::string name, Node::Stmt *value)
      : line(line), pos(pos), name(name), value(value) {
    kind = NodeKind::ND_CONST_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ConstStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(ident + 1);
    std::cout << "Value: \n";
    value->debug(ident + 2);
  }

  ~ConstStmt() { delete value; }
};

class BlockStmt : public Node::Stmt {
public:
  int line, pos;
  std::vector<Node::Stmt *> stmts;

  BlockStmt(int line, int pos, std::vector<Node::Stmt *> stmts)
      : line(line), pos(pos), stmts(stmts) {
    kind = NodeKind::ND_BLOCK_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "BlockStmt: \n";
    for (auto s : stmts) {
      s->debug(ident + 1);
    }
    Node::printIndent(ident);
    std::cout << "End of BlockStmt\n";
  }

  ~BlockStmt() {
    for (auto s : stmts) {
      delete s;
    }
  }
};

class VarStmt : public Node::Stmt {
public:
  int line, pos;
  bool isConst;
  std::string name;
  Node::Type *type;
  ExprStmt *expr;

  VarStmt(int line, int pos, bool isConst, std::string name, Node::Type *type,
          ExprStmt *expr)
      : line(line), pos(pos), isConst(isConst), name(name), type(type),
        expr(expr) {
    kind = NodeKind::ND_VAR_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "VarStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "IsConst: " << isConst << "\n";
    Node::printIndent(ident + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(ident + 1);
    std::cout << "Type: ";
    type->debug(ident + 2);
    std::cout << "\n";
    if (expr) {
      Node::printIndent(ident + 1);
      std::cout << "Expr: \n";
      expr->debug(ident + 2);
    }
  }

  ~VarStmt() {
    delete expr;
    delete type;
  }
};

class PrintStmt : public Node::Stmt {
public:
  int line, pos;
  std::vector<Node::Expr *> args;

  PrintStmt(int line, int pos, std::vector<Node::Expr *> args)
      : line(line), pos(pos), args(args) {
    kind = NodeKind::ND_PRINT_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "PrintStmt: \n";
    for (auto a : args) {
      a->debug(ident + 1);
    }
  }

  ~PrintStmt() {
    for (auto a : args) {
      delete a;
    }
  }
};

class fnStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::pair<std::string, Node::Type *>> params;
  Node::Type *returnType;
  Node::Stmt *block;
  bool isMain = false;

  fnStmt(int line, int pos, std::string name,
         std::vector<std::pair<std::string, Node::Type *>> params,
         Node::Type *returnType, Node::Stmt *block, bool isMain = false)
      : line(line), pos(pos), name(name), params(params),
        returnType(returnType), block(block), isMain(isMain) {
    kind = NodeKind::ND_FN_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "fnStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(ident + 1);
    std::cout << "Params: \n";
    for (auto p : params) {
      Node::printIndent(ident + 2);
      std::cout << "Name: " << p.first << ", Type: ";
      p.second->debug(ident + 2);
      std::cout << "\n";
    }
    Node::printIndent(ident + 1);
    std::cout << "ReturnType: ";
    returnType->debug(ident);
    std::cout << "\n";
    block->debug(ident + 1);
  }

  ~fnStmt() {
    delete block;
    for (auto p : params) {
      delete p.second;
    }
    delete returnType;
  }
};

class ReturnStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *expr;

  ReturnStmt(int line, int pos, Node::Expr *expr)
      : line(line), pos(pos), expr(expr) {
    kind = NodeKind::ND_RETURN_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ReturnStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Expr: \n";
    expr->debug(ident + 2);
  }

  ~ReturnStmt() { delete expr; }
};

class IfStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Stmt *thenStmt;
  Node::Stmt *elseStmt;

  IfStmt(int line, int pos, Node::Expr *condition, Node::Stmt *thenStmt,
         Node::Stmt *elseStmt)
      : line(line), pos(pos), condition(condition), thenStmt(thenStmt),
        elseStmt(elseStmt) {
    kind = NodeKind::ND_IF_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "IfStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Condition: \n";
    condition->debug(ident + 2);
    Node::printIndent(ident + 1);
    std::cout << "ThenStmt: \n";
    thenStmt->debug(ident + 2);
    if (elseStmt) {
      Node::printIndent(ident + 1);
      std::cout << "ElseStmt: \n";
      elseStmt->debug(ident + 2);
    }
  }

  ~IfStmt() {
    delete condition;
    delete thenStmt;
    delete elseStmt;
  }
};

class StructStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::pair<std::string, Node::Type *>> fields;
  std::vector<Node::Stmt *> stmts;

  StructStmt(int line, int pos, std::string name,
             std::vector<std::pair<std::string, Node::Type *>> fields,
             std::vector<Node::Stmt *> stmts)
      : line(line), pos(pos), name(name), fields(fields), stmts(stmts) {
    kind = NodeKind::ND_STRUCT_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "StructStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(ident + 1);
    std::cout << "Fields: \n";
    for (auto f : fields) {
      Node::printIndent(ident + 2);
      std::cout << "Name: " << f.first << ", Type: ";
      f.second->debug(ident + 2);
      std::cout << "\n";
    }
    if (stmts.size() > 0) {
      Node::printIndent(ident + 1);
      std::cout << "Stmts: \n";
      for (auto s : stmts) {
        s->debug(ident + 2);
      }
    }
  }

  ~StructStmt() {
    for (auto f : fields) {
      delete f.second;
    }
    for (auto s : stmts) {
      delete s;
    }
  }
};

class WhileStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Expr *optional;
  Node::Stmt *block;

  WhileStmt(int line, int pos, Node::Expr *condition, Node::Expr *optional,
            Node::Stmt *block)
      : line(line), pos(pos), condition(condition), block(block),
        optional(optional) {
    kind = NodeKind::ND_WHILE_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "WhileStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Condition: \n";
    condition->debug(ident + 2);
    Node::printIndent(ident + 1);
    if (optional) {
      std::cout << "Optional: \n";
      optional->debug(ident + 2);
      Node::printIndent(ident + 1);
    }
    std::cout << "Block: \n";
    block->debug(ident + 2);
  }

  ~WhileStmt() {
    delete optional;
    delete condition;
    delete block;
  }
};

class ForStmt : public Node::Stmt {
public:
  int line, pos;
  std::string varName;
  Node::Expr *forLoop;
  Node::Expr *optional;
  Node::Stmt *block;

  ForStmt(int line, int pos, std::string varName, Node::Expr *forLoop,
          Node::Expr *optional, Node::Stmt *block)
      : line(line), pos(pos), varName(varName), forLoop(forLoop),
        optional(optional), block(block) {
    kind = NodeKind::ND_FOR_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ForStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "VarName: " << varName << "\n";
    Node::printIndent(ident + 1);
    std::cout << "ForLoop: \n";
    forLoop->debug(ident + 2);
    Node::printIndent(ident + 1);
    if (optional) {
      std::cout << "Optional: \n";
      optional->debug(ident + 2);
      Node::printIndent(ident + 1);
    }
    std::cout << "Block: \n";
    block->debug(ident + 2);
  }

  ~ForStmt() {
    delete forLoop;
    delete optional;
    delete block;
  }
};

class EnumStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::string> fields;

  EnumStmt(int line, int pos, std::string name, std::vector<std::string> fields)
      : line(line), pos(pos), name(name), fields(fields) {
    kind = NodeKind::ND_ENUM_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "EnumStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(ident + 1);
    std::cout << "Fields: \n";
    for (auto f : fields) {
      Node::printIndent(ident + 2);
      std::cout << f << "\n";
    }
  }
};

class ImportStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  Node::Stmt *stmt;

  ImportStmt(int line, int pos, std::string name, Node::Stmt *stmt) : line(line), pos(pos), name(name), stmt(stmt) {
    kind = NodeKind::ND_IMPORT_STMT;
  }

  void debug(int ident = 0) const override {
    Node::printIndent(ident);
    std::cout << "ImportStmt: \n";
    Node::printIndent(ident + 1);
    std::cout << "Name: " << name << "\n";
    stmt->debug(ident + 1);
  }

  ~ImportStmt() {
    delete stmt;
  }
};