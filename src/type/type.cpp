#include <cstring>
#include <iostream>

#include "../helper/error/error.hpp"
#include "../ast/ast.hpp"
#include "type.hpp"

void Type::checkExpression(AstNode *expr) {
  if (expr->type == AstNodeType::NUMBER_LITERAL) {
    AstNode::NumberLiteral *numberLiteral =
        (AstNode::NumberLiteral *)expr->data;

    determineType(numberLiteral->value);
  }
  if (expr->type == AstNodeType::STRING_LITERAL) {
    returnType = findType["str"];
  }
}

void Type::checkBody(AstNode *body) {
  if (body->type == AstNodeType::BLOCK) {
    AstNode::Block *block = (AstNode::Block *)body->data;

    for (AstNode *stmt : block->statements) {
      switch (stmt->type) {
      case AstNodeType::EXIT: {
        AstNode::Exit *exit = (AstNode::Exit *)stmt->data;
        checkExpression(exit->expression);
        break;
      }
      case AstNodeType::RETURN: {
        AstNode::Return *return_ = (AstNode::Return *)stmt->data;
        checkExpression(return_->expression);
        break;
      }
      default:
        break;
      }
    }
  }
}

void Type::typeCheck(AstNode *expression) {
  if (expression->type == AstNodeType::PROGRAM) {
    AstNode::Program *program = (AstNode::Program *)expression->data;

    for (AstNode *stmt : program->statements) {
      switch (stmt->type) {
      case AstNodeType::FUNCTION_DECLARATION: {
        AstNode::FunctionDeclaration *functionDeclaration =
            (AstNode::FunctionDeclaration *)stmt->data;

        functionDeclaration->name.start =
            strtok(const_cast<char *>(functionDeclaration->name.start), "(");
        name = functionDeclaration->name.start;

        AstNode::Type *type_ = (AstNode::Type *)functionDeclaration->type->data;
        type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
        type = type_;

        checkBody(functionDeclaration->body);
        break;
      }
      default: break; }

      Error::errorType(type, returnType, name);
      returnType = nullptr;
      type = nullptr;
      name = "";
    }
    free(type);
    free(returnType);
    return;
  }
}
