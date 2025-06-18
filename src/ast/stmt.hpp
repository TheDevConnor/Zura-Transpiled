#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "ast.hpp"
#include "../lexer/lexer.hpp"

class ProgramStmt : public Node::Stmt {
public:
  std::vector<Node::Stmt *> stmt; // vector of stmts - the body
  std::string inputPath;          // yes this is actually useful trust me

  ProgramStmt(std::vector<Node::Stmt *> stmt, std::string path)
      : stmt(stmt), inputPath(path) {
    kind = NodeKind::ND_PROGRAM;
    file_id = 0; // This is the main file, no? It would make no sense
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ProgramStmt: \n";
    for (Node::Stmt *s : stmt) {
      s->debug(indent + 1);
    }
  }

  ~ProgramStmt() = default; // rule of threes
};

class MatchStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *coverExpr;
  std::vector<std::pair<Node::Expr *, Node::Stmt *>> cases;
  Node::Stmt *defaultCase;

  MatchStmt(int line, int pos, Node::Expr *coverExpr,
            std::vector<std::pair<Node::Expr *, Node::Stmt *>> cases,
            Node::Stmt *defaultCase, size_t file)
      : line(line), pos(pos), coverExpr(coverExpr), cases(cases),
        defaultCase(defaultCase) {
    file_id = file;
    kind = NodeKind::ND_MATCH_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "MatchStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "WhatExpr: \n";
    coverExpr->debug(indent + 2);
    for (size_t i = 0; i < cases.size(); i++) {
      Node::printIndent(indent + 1);
      std::cout << "Case #" << std::to_string(i) << ": \n";
      Node::printIndent(indent + 2);
      std::cout << "Expr: \n";
      cases[i].first->debug(indent + 3);
      Node::printIndent(indent + 2);
      std::cout << "Stmt: \n";
      cases[i].second->debug(indent + 3);
    }
    if (defaultCase) {
      Node::printIndent(indent + 1);
      std::cout << "Default Case: \n";
      defaultCase->debug(indent + 2);
    }
  }
};

class ExprStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *expr;

  ExprStmt(int line, int pos, Node::Expr *expr, size_t file)
      : line(line), pos(pos), expr(expr) {
    file_id = file;
    kind = NodeKind::ND_EXPR_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ExprStmt: \n";
    expr->debug(indent + 1);
  }

  ~ExprStmt() = default; // rule of threes
};

class ConstStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  Node::Stmt *value;

  ConstStmt(int line, int pos, std::string name, Node::Stmt *value, size_t file)
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

  ~ConstStmt() = default; // rule of threes
};

class BlockStmt : public Node::Stmt {
public:
  int line, pos;
  std::vector<Node::Stmt *> stmts;
  std::vector<Node::Type *> varDeclTypes; // If there are 2 int declarations in
                                          // this scope, this will be 16.
  bool shouldDeclareForward;

  BlockStmt(int line, int pos, std::vector<Node::Stmt *> stmts,
            bool declareForward, std::vector<Node::Type *> varDeclTypes,
            size_t file)
      : line(line), pos(pos), stmts(stmts),
        varDeclTypes(std::move(varDeclTypes)),
        shouldDeclareForward(declareForward) {
    file_id = file;
    kind = NodeKind::ND_BLOCK_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "BlockStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "ShouldDeclareForward: "
              << (shouldDeclareForward ? "Yes" : "No") << "\n";
    Node::printIndent(indent + 1);
    std::cout << "VarDeclTypes: \n";
    for (Node::Type *t : varDeclTypes) {
      Node::printIndent(indent + 2);
      t->debug(indent + 2);
    }
    // new line automatically printed
    Node::printIndent(indent + 1);
    std::cout << "Body: \n";
    for (Node::Stmt *s : stmts) {
      s->debug(indent + 2);
    }
    Node::printIndent(indent);
    std::cout << "End of BlockStmt\n";
  }

  ~BlockStmt() = default; // rule of threes
};

class VarStmt : public Node::Stmt {
public:
  int line, pos;
  bool isConst;
  std::string name;
  Node::Type *type;
  Node::Expr *expr;

  VarStmt(int line, int pos, bool isConst, std::string name, Node::Type *type,
          Node::Expr *expr, size_t file)
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
    Node::printIndent(indent + 1);
    type->debug(indent + 2);
    std::cout << "\n";
    if (expr) {
      Node::printIndent(indent + 1);
      std::cout << "Expr: \n";
      expr->debug(indent + 2);
    }
  }

  ~VarStmt() = default; // rule of threes
};

class OutputStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *fd;
  std::vector<Node::Expr *> args;
  bool isPrintln = false;

  OutputStmt(int line, int pos, Node::Expr *fd, std::vector<Node::Expr *> args,
             size_t file, bool isPrintln = false)
      : line(line), pos(pos), fd(fd), args(std::move(args)),
        isPrintln(isPrintln) {
    file_id = file;
    kind = NodeKind::ND_PRINT_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "PrintStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "File descriptor: \n";
    fd->debug(indent + 2);
    for (Node::Expr *a : args) {
      Node::printIndent(indent + 1);
      a->debug(indent + 1);
    }
  }

  ~OutputStmt() = default; // rule of threes
};
class FnStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::string> typenames;
  std::vector<std::pair<std::string, Node::Type *>> params;
  Node::Type *returnType;
  Node::Stmt *block;
  bool isMain = false;
  bool isEntry = false;
  bool isTemplate = false;

  FnStmt(int line, int pos, std::string name,
         std::vector<std::pair<std::string, Node::Type *>> params,
         Node::Type *returnType, Node::Stmt *block,
         std::vector<std::string> typenames, bool isMain = false,
         bool isEntry = false, bool isTemplate = false, size_t file = 0)
      : line(line), pos(pos), name(name), typenames(typenames),
        params(std::move(params)), returnType(returnType), block(block),
        isMain(isMain), isEntry(isEntry), isTemplate(isTemplate) {
    file_id = file;
    kind = NodeKind::ND_FN_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "FnStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "IsMain: " << (isMain ? "Yes" : "No") << "\n";
    Node::printIndent(indent + 1);
    std::cout << "IsEntry: " << (isEntry ? "Yes" : "No") << "\n";
    Node::printIndent(indent + 1);
    std::cout << "IsTemplate: " << (isTemplate ? "Yes" : "No") << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Typenames: \n";
    for (std::string t : typenames) {
      Node::printIndent(indent + 2);
      std::cout << t << "\n";
    }
    Node::printIndent(indent + 1);
    std::cout << "Params: \n";
    for (std::pair<std::string, Node::Type *> p : params) {
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

  ~FnStmt() = default; // rule of threes
};

class ReturnStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *expr;

  ReturnStmt(int line, int pos, Node::Expr *expr, size_t file = 0)
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

  ~ReturnStmt() = default; // rule of threes
};

class IfStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Stmt *thenStmt;
  Node::Stmt *elseStmt;

  IfStmt(int line, int pos, Node::Expr *condition, Node::Stmt *thenStmt,
         Node::Stmt *elseStmt, size_t file)
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

  ~IfStmt() = default; // rule of threes
};

class StructStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::string> typenames;
  std::vector<std::pair<std::string, Node::Type *>> fields;
  std::vector<Node::Stmt *> stmts;
  bool isTemplate = false;

  StructStmt(int line, int pos, std::string name,
             std::vector<std::pair<std::string, Node::Type *>> fields,
             std::vector<Node::Stmt *> stmts,
             std::vector<std::string> typenames, size_t file,
             bool isTemplate = false)
      : line(line), pos(pos), name(name), typenames(typenames), fields(fields),
        stmts(stmts), isTemplate(isTemplate) {
    file_id = file;
    kind = NodeKind::ND_STRUCT_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "StructStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "Name: " << name << "\n";
    Node::printIndent(indent + 1);
    std::cout << "Typenames: \n";
    for (std::string t : typenames) {
      Node::printIndent(indent + 2);
      std::cout << t << "\n";
    }
    std::cout << "\n";
    std::cout << "Fields: \n";
    for (std::pair<std::string, Node::Type *> f : fields) {
      Node::printIndent(indent + 2);
      std::cout << "Name: " << f.first << ", \n";
      f.second->debug(indent + 2);
      std::cout << "\n";
    }
    if (stmts.size() > 0) {
      Node::printIndent(indent + 1);
      std::cout << "Stmts: \n";
      for (Node::Stmt *s : stmts) {
        s->debug(indent + 2);
      }
    }
  }

  ~StructStmt() = default; // rule of threes
};

class WhileStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *condition;
  Node::Expr *optional;
  Node::Stmt *block;

  WhileStmt(int line, int pos, Node::Expr *condition, Node::Expr *optional,
            Node::Stmt *block, size_t file)
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

  ~WhileStmt() = default; // rule of threes
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
          Node::Expr *condition, Node::Expr *optional, Node::Stmt *block,
          size_t file)
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

  ~ForStmt() = default; // rule of threes
};

class EnumStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  std::vector<std::string> fields;

  EnumStmt(int line, int pos, std::string name, std::vector<std::string> fields,
           size_t file)
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
    for (std::string f : fields) {
      Node::printIndent(indent + 2);
      std::cout << f << "\n";
    }
  }
};

class ImportStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;
  Node::Stmt *stmt;
  std::vector<Lexer::Token> tks;

  ImportStmt(int line, int pos, std::string name, Node::Stmt *stmt, std::vector<Lexer::Token> tks, size_t file)
      : line(line), pos(pos), name(name), stmt(stmt), tks(std::move(tks)) {
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

  ~ImportStmt() = default; // rule of threes
};

class LinkStmt : public Node::Stmt {
public:
  int line, pos;
  std::string name;

  LinkStmt(int line, int pos, std::string name, size_t file)
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
  std::vector<std::string> externs;

  ExternStmt(int line, int pos, std::string name,
             std::vector<std::string> externs, size_t file)
      : line(line), pos(pos), name(name), externs(externs) {
    file_id = file;
    kind = NodeKind::ND_EXTERN_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ExternStmt: \n";
    Node::printIndent(indent + 1);
    if (externs.size() > 0) {
      std::cout << "Externs: \n";
      for (std::string e : externs) {
        Node::printIndent(indent + 2);
        std::cout << e << "\n";
      }
    } else {
      std::cout << "Name: " << name << "\n";
    }
  }
};

class BreakStmt : public Node::Stmt {
public:
  int line, pos;

  BreakStmt(int line, int pos, size_t file) : line(line), pos(pos) {
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

  ContinueStmt(int line, int pos, size_t file) : line(line), pos(pos) {
    file_id = file;
    kind = NodeKind::ND_CONTINUE_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "ContinueStmt\n";
  }
};

class InputStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *bufferOut;
  Node::Expr *maxBytes;
  Node::Expr *fd;

  InputStmt(int line, int pos, Node::Expr *fd, Node::Expr *bufferOut,
            Node::Expr *sysCall, size_t file)
      : line(line), pos(pos), bufferOut(bufferOut), maxBytes(sysCall), fd(fd) {
    file_id = file;
    kind = NodeKind::ND_INPUT_STMT;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "InputStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "FD: \n";
    fd->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Output buffer: \n";
    bufferOut->debug(indent + 2);
    Node::printIndent(indent + 1);
    std::cout << "Input byte count: \n";
    maxBytes->debug(indent + 2);
  }
};

class CloseStmt : public Node::Stmt {
public:
  int line, pos;
  Node::Expr *fd;

  CloseStmt(int line, int pos, Node::Expr *fd, size_t file)
      : line(line), pos(pos), fd(fd) {
    file_id = file;
    kind = NodeKind::ND_CLOSE;
  }

  void debug(int indent = 0) const override {
    Node::printIndent(indent);
    std::cout << "CloseStmt: \n";
    Node::printIndent(indent + 1);
    std::cout << "FD: \n";
    fd->debug(indent + 2);
  }
};
