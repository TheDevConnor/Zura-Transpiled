#pragma once

#include "ast.hpp"

#include <iostream>
#include <string>
#include <vector>

class ProgramStmt : public Node::Stmt {
public:
  std::vector<Node::Stmt *> stmt; // vector of stmts - the body
  std::string inputPath; // yes this is actually useful trust me

  ProgramStmt(std::vector<Node::Stmt *> stmt, std::string path) : stmt(stmt), inputPath(path) {
    kind = NodeKind::ND_PROGRAM;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ProgramStmt: \n";
    for (auto s : stmt) {
      s->debug(indent + 1);
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

  ExprStmt(int line, int pos, Node::Expr *expr, int file)
      : line(line), pos(pos), expr(expr) {
    file_id = file;
    kind = NodeKind::ND_EXPR_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ExprStmt: \n";
    expr->debug(indent + 1);
  }

  ~ExprStmt() { delete expr; }
};

class ConstStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  Node::Stmt *value;

  ConstStmt(int line, int pos, std::string name, Node::Stmt *value, int file)
      : line(line), pos(pos), name(name), value(value) {
    file_id = file;
    kind = NodeKind::ND_CONST_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ConstStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Value: \n";
    value->debug(indent + 1);
  }

  ~ConstStmt() { delete value; }
};

class BlockStmt : public Node::Stmt {
public:
  int line, pos;
  std::vector<Node::Stmt *> stmts;

  BlockStmt(int line, int pos, std::vector<Node::Stmt *> stmts, int file)
      : line(line), pos(pos), stmts(stmts) {
    file_id = file;
    kind = NodeKind::ND_BLOCK_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "BlockStmt: \n";
    for (auto s : stmts) {
      s->debug(indent + 1);
    }
    Node::printIndent(indent);
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
          ExprStmt *expr, int file)
      : line(line), pos(pos), isConst(isConst), name(name), type(type),
        expr(expr) {
    file_id = file;
    kind = NodeKind::ND_VAR_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "VarStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "IsConst: " << isConst << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Type: \n";
    Node::printIndent(indent + 2);
    type->debug(indent + 2);
    std::cout << "\n";
    if (expr) {
      Node::printIndent(indent + 1);
      std::cout << "Expr: \n";
      expr->debug(indent + 2);
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

  PrintStmt(int line, int pos, std::vector<Node::Expr *> args, int file)
      : line(line), pos(pos), args(std::move(args)) {
    file_id = file;
    kind = NodeKind::ND_PRINT_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "PrintStmt: \n";
    for (auto a : args) {
      a->debug(indent + 1);
    }
  }

  ~PrintStmt() {
    for (auto a : args) {
      delete a;
    }
  }
};
class FnStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::pair<std::string, Node::Type *>> params;
  Node::Type *returnType;
  Node::Stmt *block;
  bool isMain = false;
  bool isEntry = false;

  FnStmt(int line, int pos, std::string name,
         std::vector<std::pair<std::string, Node::Type *>> params,
         Node::Type *returnType, Node::Stmt *block, bool isMain = false,
         bool isEntry = false, int file = 0)
      : line(line), pos(pos), name(name), params(std::move(params)),
        returnType(returnType), block(block), isMain(isMain), isEntry(isEntry) {
    file_id = file;
    kind = NodeKind::ND_FN_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "FnStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Params: \n";
    for (auto p : params) {
      Node::printIndent(indent + 2);
      std::cout << "Name: " << p.first << "\n";
      Node::printIndent(indent + 3);
      std::cout << "Type: \n";
      Node::printIndent(indent + 4);
      p.second->debug(indent + 4);
      std::cout << "\n";
    }
    Node::printIndent(indent + 1);
    std::cout << "ReturnType: \n";
    Node::printIndent(indent + 2);
    returnType->debug(indent + 2);
    std::cout << "\n";
    block->debug(indent + 1);
  }

  ~FnStmt() {
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

  ReturnStmt(int line, int pos, Node::Expr *expr, int file = 0)
      : line(line), pos(pos), expr(expr) {
    file_id = file;
    kind = NodeKind::ND_RETURN_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ReturnStmt: \n";
    Node::printIndent(indent + 1);
    if (expr) {
      std::cout << "Expr: \n";
      expr->debug(indent + 2);
    }
  }

  ~ReturnStmt() {
    delete expr;
  }
};

class IfStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Stmt *thenStmt;
  Node::Stmt *elseStmt;

  IfStmt(int line, int pos, Node::Expr *condition, Node::Stmt *thenStmt,
         Node::Stmt *elseStmt, int file)
      : line(line), pos(pos), condition(condition), thenStmt(thenStmt),
        elseStmt(elseStmt) {
    file_id = file;
    kind = NodeKind::ND_IF_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "IfStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Condition: \n";
    condition->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "ThenStmt: \n";
    thenStmt->debug(indent + 2);
    if (elseStmt) {
      Node::printIndent(indent + 1);
      std::cout << "ElseStmt: \n";
      elseStmt->debug(indent + 2);
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
             std::vector<Node::Stmt *> stmts, int file)
      : line(line), pos(pos), name(name), fields(fields), stmts(stmts) {
    file_id = file;
    kind = NodeKind::ND_STRUCT_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "StructStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Fields: \n";
    for (auto f : fields) {
      Node::printIndent(indent + 2);
      std::cout << "Name: " << f.first << ", Type: ";
      f.second->debug(indent + 2);
      std::cout << "\n";
    }
    if (stmts.size() > 0) {
      Node::printIndent(indent + 1);
      std::cout << "Stmts: \n";
      for (auto s : stmts) {
        s->debug(indent + 2);
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
            Node::Stmt *block, int file)
      : line(line), pos(pos), condition(condition), optional(optional),
       block(block) {
    file_id = file;
    kind = NodeKind::ND_WHILE_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "WhileStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Condition: \n";
    condition->debug(indent + 2);
    Node::printIndent(indent + 1);
    if (optional) {
      std::cout << "Optional: \n";
      optional->debug(indent + 2);
      Node::printIndent(indent + 1);
    }
    std::cout << "Block: \n";
    block->debug(indent + 2);
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
  std::string name;
  Node::Expr *forLoop;
  Node::Expr *condition;
  Node::Expr *optional;
  Node::Stmt *block;

  ForStmt(int line, int pos, std::string name, Node::Expr *forLoop,
          Node::Expr *condition, Node::Expr *optional, Node::Stmt *block, int file)
      : line(line), pos(pos), name(name), forLoop(forLoop),
        condition(condition), optional(optional), block(block) {
    file_id = file;
    kind = NodeKind::ND_FOR_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ForStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "ForLoop: \n";
    forLoop->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Condition: \n";
    condition->debug(indent + 2);
    Node::printIndent(indent + 1);
    if (optional) {
      std::cout << "Optional: \n";
      optional->debug(indent + 2);
      Node::printIndent(indent + 1);
    }
    std::cout << "Block: \n";
    block->debug(indent + 2);
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

  EnumStmt(int line, int pos, std::string name, std::vector<std::string> fields, int file)
      : line(line), pos(pos), name(name), fields(fields) {
    file_id = file;
    kind = NodeKind::ND_ENUM_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "EnumStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Fields: \n";
    for (auto f : fields) {
      Node::printIndent(indent + 2);
      std::cout << f << "\n";
    }
  }
};

class TemplateStmt : public Node::Stmt {
public:
  std::vector<std::string> typenames;
  int line, pos;

  TemplateStmt(std::vector<std::string> typenames, int line, int pos, int file)
      : typenames(typenames), line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_TEMPLATE_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "TemplateStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Typenames: \n";
    for (auto t : typenames) {
      Node::printIndent(indent + 2);
      std::cout << t << "\n";
    }
  }

  ~TemplateStmt() {}
};

class ImportStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  Node::Stmt *stmt;

  ImportStmt(int line, int pos, std::string name, Node::Stmt *stmt, int file)
      : line(line), pos(pos), name(name), stmt(stmt) {
    file_id = file;
    kind = NodeKind::ND_IMPORT_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ImportStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    stmt->debug(indent + 1);
  }

  ~ImportStmt() { delete stmt; }
};

class LinkStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;

  LinkStmt(int line, int pos, std::string name, int file)
      : line(line), pos(pos), name(name) {
    file_id = file;
    kind = NodeKind::ND_LINK_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "LinkStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
  }
};

class ExternStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;

  ExternStmt(int line, int pos, std::string name, int file)
      : line(line), pos(pos), name(name) {
    file_id = file;
    kind = NodeKind::ND_EXTERN_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ExternStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
  }
};

class BreakStmt : public Node::Stmt {
public:
  int line, pos;

  BreakStmt(int line, int pos, int file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_BREAK_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "BreakStmt\n";
  }
};

class ContinueStmt : public Node::Stmt {
public:
  int line, pos;

  ContinueStmt(int line, int pos, int file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_CONTINUE_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ContinueStmt\n";
  }
};
