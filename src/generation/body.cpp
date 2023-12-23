#include <cstring>
#include <fstream>
#include <iostream>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "gen.hpp"

void Gen::prologue(std::ofstream &file) {
  file << "  ; Prologue\n";
  file << "  push ebp\n";
  file << "  mov ebp, esp\n\n";
}

void Gen::epilogue(std::ofstream &file) {
  file << "  ; Epilogue\n";
  file << "  pop ebp\n";
  file << "  ret\n";
}

void Gen::findBinaryOp(std::ofstream &file, AstNode::Binary *op) {
  AstNode::Binary *binary = (AstNode::Binary *)op;
  switch (binary->op) {
  case TokenKind::PLUS:
    file << " + ";
    break;
  case TokenKind::MINUS:
    file << " - ";
    break;
  case TokenKind::STAR:
    file << " * ";
    break;
  case TokenKind::SLASH:
    file << " / ";
    break;
  case TokenKind::MODULO:
    file << " % ";
    break;
  case TokenKind::EQUAL_EQUAL:
    file << " == ";
    break;
  case TokenKind::BANG_EQUAL:
    file << " != ";
    break;
  case TokenKind::GREATER:
    file << " > ";
    break;
  case TokenKind::GREATER_EQUAL:
    file << " >= ";
    break;
  case TokenKind::LESS:
    file << " < ";
    break;
  case TokenKind::LESS_EQUAL:
    file << " <= ";
    break;
  default:
    break;
  }
}

void Gen::expression(std::ofstream &file, AstNode *node) {
  switch (node->type) {
  case AstNodeType::CALL: {
    AstNode::Call *call = static_cast<AstNode::Call *>(node->data);
    call->callee->type = AstNodeType::IDENTIFIER;
    expression(file, call->callee);
    file << "(";
    for (AstNode *arg : call->arguments) {
      expression(file, arg);
      file << ", ";
    }
    if (call->arguments.size() > 0)
      file.seekp(-2, std::ios_base::end);
    file << ")";
    break;
  }
  case AstNodeType::IDENTIFIER: {
    AstNode::Identifier *identifier =
        static_cast<AstNode::Identifier *>(node->data);
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), " ");
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ",");
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), "(");
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ")");
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ";");
    file << identifier->name.start;
    break;
  }
  case AstNodeType::NUMBER_LITERAL: {
    AstNode::NumberLiteral *number =
        static_cast<AstNode::NumberLiteral *>(node->data);
    file << number->value;
    break;
  }
  case AstNodeType::STRING_LITERAL: {
    AstNode::StringLiteral *string =
        static_cast<AstNode::StringLiteral *>(node->data);
    string->value = strtok(const_cast<char *>(string->value.c_str()), "\"");
    file << string->value;
    break;
  }
  case AstNodeType::BINARY: {
    AstNode::Binary *binary = static_cast<AstNode::Binary *>(node->data);
    expression(file, binary->left);
    findBinaryOp(file, binary);
    expression(file, binary->right);
    break;
  }
  case AstNodeType::UNARY: {
    AstNode::Unary *unary = static_cast<AstNode::Unary *>(node->data);
    file << unary->op;
    expression(file, unary->right);
    break;
  }
  case AstNodeType::GROUPING: {
    AstNode::Grouping *grouping = static_cast<AstNode::Grouping *>(node->data);
    file << "(";
    expression(file, grouping->expression);
    file << ")";
    break;
  }
  default:
    break;
  }
}

void Gen::callStmt(std::ofstream &file, AstNode *node) {
  // TODO: Update to asm
  // AstNode::Call *call = static_cast<AstNode::Call *>(node->data);
  // call->callee->type = AstNodeType::IDENTIFIER;
  // expression(file, call->callee);
  // file << "(";
  // for (AstNode *arg : call->arguments) {
  //   expression(file, arg);
  //   file << ", ";
  // }
  // file << ");\n";
}

void Gen::printStmt(std::ofstream &file, AstNode *node) {
  AstNode::Print *print = static_cast<AstNode::Print *>(node->data);

  while (globalVar == true) {
    file << "\t; Callthe global var\n";
    file << "\tmovzx eax, byte [x] \n";
    file << "\tpush eax";
    globalVar = false;
  }

  file << "\n";
  file << "  ; Call printf \n";
  file << "  push dword fmt\n";
  file << "  call printf\n";
  file << "  add esp, 8\n\n";
}

void Gen::exitStmt(std::ofstream &file, AstNode *node) {
  AstNode::Exit *exit = static_cast<AstNode::Exit *>(node->data);
  if (exit->expression->type == AstNodeType::NUMBER_LITERAL) {
    AstNode::NumberLiteral *number =
        (AstNode::NumberLiteral *)exit->expression->data;
    file << "  push " << number->value << "\n";
    file << "  call exit\n\n";
  } else if (exit->expression->type == AstNodeType::IDENTIFIER) {
    AstNode::Identifier *identifier =
        (AstNode::Identifier *)exit->expression->data;
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ";");
    file << "  push " << identifier->name.start << "\n";
    file << "  call exit\n\n";
  }
}

void Gen::returnStmt(std::ofstream &file, AstNode *node) {
  // TODO: Update to asm
  // AstNode::Return *returnStmt = static_cast<AstNode::Return *>(node->data);
  // file << "  return ";
  // expression(file, returnStmt->expression);
  // file << ";\n";
}

void Gen::varDeclaration(std::ofstream &file, AstNode *node, int indent) {
  // TODO: Update to asm
  // AstNode::VarDeclaration *var =
  //     static_cast<AstNode::VarDeclaration *>(node->data);

  // for (int i = 0; i < indent; i++)
  //   file << " ";
  // const char *type = findType(var->type);
  // printTypeToFile(file, type);

  // var->name.start = strtok(const_cast<char *>(var->name.start), "<");
  // file << var->name.start << "= ";

  // expression(file, var->initializer);
  // file << ";\n";
}

void Gen::blockStmt(std::ofstream &file, AstNode *node) {
  AstNode::Block *block = static_cast<AstNode::Block *>(node->data);
  for (AstNode *stmt : block->statements) {
    switch (stmt->type) {
    case AstNodeType::FUNCTION_DECLARATION:
      functionDeclaration(file, stmt);
      break;
    case AstNodeType::VAR_DECLARATION:
      varDeclaration(file, stmt, 2);
      break;
    case AstNodeType::RETURN:
      returnStmt(file, stmt);
      break;
    case AstNodeType::PRINT:
      printStmt(file, stmt);
      break;
    case AstNodeType::EXIT:
      exitStmt(file, stmt);
      break;
    case AstNodeType::BLOCK:
      blockStmt(file, stmt);
      break;
    case AstNodeType::CALL:
      callStmt(file, stmt);
      break;
    default:
      break;
    }
  }
}

void Gen::body(std::ofstream &file, AstNode *node) {
  if (ast->type == AstNodeType::PROGRAM) {
    AstNode::Program *program = static_cast<AstNode::Program *>(ast->data);

    secData(file, program);
    secText(file);

    for (AstNode *node : program->statements) {
      switch (node->type) {
      case AstNodeType::FUNCTION_DECLARATION:
        functionDeclaration(file, node);
        break;
      case AstNodeType::PRINT:
        printStmt(file, node);
        break;
      case AstNodeType::VAR_DECLARATION:
        varDeclaration(file, node, 0);
        break;
      case AstNodeType::BLOCK:
        blockStmt(file, node);
        break;
      case AstNodeType::CALL:
        callStmt(file, node);
        break;
      default:
        break;
      }
    }
  }
}
