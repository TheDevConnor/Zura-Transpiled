#include <cstring>
#include <iostream>

#include "../ast/ast.hpp"
#include "../common.hpp"
#include "type.hpp"

void Type::checkExpression(AstNode *expr) {
  if (expr->type == AstNodeType::NUMBER_LITERAL) {
    AstNode::NumberLiteral *numberLiteral =
        (AstNode::NumberLiteral *)expr->data;
    returnType = determineType(numberLiteral->value);
  }

  if (expr->type == AstNodeType::STRING_LITERAL) {
    std::cout << "StringLiteral" << std::endl;
    AstNode::StringLiteral *stringLiteral =
        (AstNode::StringLiteral *)expr->data;
    returnType = findType(stringLiteral->value);
  }

  if (expr->type == AstNodeType::IDENTIFIER) {
    AstNode::Identifier *identifier = (AstNode::Identifier *)expr->data;
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), " ");

    for (auto &param : paramTypes) {
      if (strcmp(identifier->name.start, param.first.c_str()) == 0) {
        returnType = findType(param.second);
        return;
      }
    }

    if (returnType == nullptr) {
      std::cout << "Error: " << identifier->name.start
                << " is not defined in the current scope" << std::endl;
      Exit(ExitValue::TYPE_ERROR);
    }
  }

  if (expr->type == AstNodeType::BINARY) {
    AstNode::Binary *binary = (AstNode::Binary *)expr->data;
    checkExpression(binary->left);
    checkExpression(binary->right);
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
      case AstNodeType::VAR_DECLARATION: {
        AstNode::VarDeclaration *varDeclaration =
            (AstNode::VarDeclaration *)stmt->data;

        varDeclaration->name.start =
            strtok(const_cast<char *>(varDeclaration->name.start), " ");
        name = varDeclaration->name.start;

        AstNode::Type *type_ = (AstNode::Type *)varDeclaration->type->data;
        type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
        type = type_;

        checkExpression(varDeclaration->initializer);
        break;
      }
      default:
        break;
      }

      checkForErrors(type, returnType, name);
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

        // get the type of the parameter
        if (functionDeclaration->parameters.size() > 0) {
          for (Lexer::Token param : functionDeclaration->parameters) {
            const char *name = strtok(const_cast<char *>(param.start), " ");
            name = strtok(const_cast<char *>(name), ":");
            const char *paramType = nullptr;

            switch (functionDeclaration->paramType.front()->type) {
            case AstNodeType::TYPE: {
              AstNode::Type *typeNode =
                  (AstNode::Type *)functionDeclaration->paramType.front()
                      ->data;
              paramType = strtok(const_cast<char *>(typeNode->type.start), ",");
              paramType = strtok(const_cast<char *>(paramType), ")");
              break;
            }
            default:
              break;
            }
            
            paramTypes.push_back(std::make_pair(name, paramType));

            functionDeclaration->paramType.erase(
              functionDeclaration->paramType.begin());
          }
        }

        AstNode::Type *type_ = (AstNode::Type *)functionDeclaration->type->data;
        type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
        type = type_;

        checkBody(functionDeclaration->body);
        break;
      }
      case AstNodeType::VAR_DECLARATION: {
        AstNode::VarDeclaration *varDeclaration =
            (AstNode::VarDeclaration *)stmt->data;

        varDeclaration->name.start =
            strtok(const_cast<char *>(varDeclaration->name.start), " ");
        name = varDeclaration->name.start;

        AstNode::Type *type_ = (AstNode::Type *)varDeclaration->type->data;
        type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
        type = type_;

        checkExpression(varDeclaration->initializer);
        break;
      }
      default:
        break;
      }
      checkForErrors(type, returnType, name);
    }
  }
}
