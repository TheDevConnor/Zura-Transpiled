#include <cstring>
#include <fstream>
#include <memory>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "gen.hpp"

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

void Gen::expression(std::ofstream &file, std::unique_ptr<AstNode> node) {
  switch (node->type) {
  case AstNodeType::IDENTIFIER: {
    AstNode::Identifier *identifier = (AstNode::Identifier *)node->data;
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ";");
    file << identifier->name.start;
    break;
  }
  case AstNodeType::NUMBER_LITERAL: {
    AstNode::NumberLiteral *number = (AstNode::NumberLiteral *)node->data;
    file << number->value;
    break;
  }
  case AstNodeType::STRING_LITERAL: {
    AstNode::StringLiteral *string = (AstNode::StringLiteral *)node->data;
    string->value = string->value.substr(1, string->value.length() - 6);
    file << string->value;
    break;
  }
  case AstNodeType::BINARY: {
    AstNode::Binary *binary = (AstNode::Binary *)node->data;
    expression(file, std::move(binary->left));
    findBinaryOp(file, binary);
    expression(file, std::move(binary->right));
    break;
  }
  case AstNodeType::UNARY: {
    AstNode::Unary *unary = (AstNode::Unary *)node->data;
    file << unary->op;
    expression(file, std::move(unary->right));
    break;
  }
  case AstNodeType::GROUPING: {
    AstNode::Grouping *grouping = (AstNode::Grouping *)node->data;
    file << "(";
    expression(file, std::move(grouping->expression));
    file << ")";
    break;
  }
  default:
    break;
  }
}

void Gen::exitStmt(std::ofstream &file, std::unique_ptr<AstNode> node) {
  AstNode::Exit *exit = (AstNode::Exit *)node->data;
  if (exit->expression->type == AstNodeType::NUMBER_LITERAL) {
    AstNode::NumberLiteral *number =
        (AstNode::NumberLiteral *)exit->expression->data;
    file << "  return " << number->value << ";\n";
  } else if (exit->expression->type == AstNodeType::IDENTIFIER) {
    AstNode::Identifier *identifier =
        (AstNode::Identifier *)exit->expression->data;
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ";");
    file << "  return " << identifier->name.start << ";\n";
  }
}

void Gen::varDeclaration(std::ofstream &file, std::unique_ptr<AstNode> node,
                         int indent) {
  AstNode::VarDeclaration *var = (AstNode::VarDeclaration *)node->data;

  for (int i = 0; i < indent; i++)
    file << " ";
  const char *type = findType(var->type.get());
  printTypeToFile(file, type);

  var->name.start = strtok(const_cast<char *>(var->name.start), "<");
  file << var->name.start << "= ";

  expression(file, std::move(var->initializer));
  file << ";\n";
}

void Gen::blockStmt(std::ofstream &file, AstNode *node) {
  AstNode::Block *block = (AstNode::Block *)node->data;
  for (AstNode *stmt : block->statements) {
    switch (stmt->type) {
    case AstNodeType::FUNCTION_DECLARATION:
      functionDeclaration(file, std::unique_ptr<AstNode>(stmt));
      break;
    case AstNodeType::VAR_DECLARATION:
      varDeclaration(file, std::unique_ptr<AstNode>(stmt), 2);
      break;
    case AstNodeType::EXIT:
      exitStmt(file, std::unique_ptr<AstNode>(stmt));
      break;
    default:
      break;
    }
  }
}

void Gen::body(std::ofstream &file, AstNode *node) {
  if (ast->type == AstNodeType::PROGRAM) {
    AstNode::Program *program = (AstNode::Program *)ast->data;

    for (AstNode *node : program->statements) {
      switch (node->type) {
      case AstNodeType::FUNCTION_DECLARATION:
        functionDeclaration(file, std::unique_ptr<AstNode>(node));
        break;
      // case AstNodeType::PRINT:                printStmt(file, node); break;
      case AstNodeType::VAR_DECLARATION:
        varDeclaration(file, std::unique_ptr<AstNode>(node), 0);
        break;
      case AstNodeType::BLOCK:
        blockStmt(file, node);
        break;
      default:
        break;
      }
    }
  }
}
