#include <cstring>
#include <iostream>

#include "../../inc/colorize.hpp"
#include "../ast/ast.hpp"
#include "../common.hpp"
#include "../lexer/lexer.hpp"
#include "type.hpp"

AstNode::Type *returnType = nullptr;
AstNode::Type *type = nullptr;
std::string name;

void Type::checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                          std::string name) {
    // string compare the type and returnType
    if (strcmp(type->type.start, returnType->type.start) == 0)
        return;

    std::cout << termcolor::red << "Error: " << termcolor::reset
              << "Expected type '" << type->type.start << "' but got '"
              << returnType->type.start << "' in function '" << name << "'"
              << std::endl;
    Exit(ExitValue::INVALID_TYPE);
}

void Type::checkExpression(AstNode *expr) {
  if (expr->type == AstNodeType::NUMBER_LITERAL) {
    AstNode::NumberLiteral *numberLiteral =
        (AstNode::NumberLiteral *)expr->data;

    // check the value of the number literal
    if (numberLiteral->value == (int)numberLiteral->value) {
      returnType = new AstNode::Type(Lexer::Token{.start = "i8"});
    } else {
      returnType = new AstNode::Type(Lexer::Token{.start = "float"});
    }
  }

  if (expr->type == AstNodeType::STRING_LITERAL) {
    AstNode::StringLiteral *stringLiteral =
        (AstNode::StringLiteral *)expr->data;
    returnType = new AstNode::Type(Lexer::Token{.start = "str"});
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
      default:
        break;
      }
      checkForErrors(type, returnType, name);
    }
  }
}
