#include "ast.hpp"
#include <cstring>
#include <iostream>

void AstNode::printVarDeclaration(AstNode::VarDeclaration *varDeclaration,
                                  int indent) {
  for (int i = 0; i < indent; i++)
    std::cout << " ";

  varDeclaration->name.start =
      strtok(const_cast<char *>(varDeclaration->name.start), "<");
  std::cout << "Name: " << varDeclaration->name.start << std::endl;

  if (varDeclaration->type) {
    for (int i = 0; i < indent; i++)
      std::cout << " ";
    std::cout << "Type: " << std::endl;
    printAst(varDeclaration->type, indent + 2);
  }

  if (varDeclaration->initializer) {
    for (int i = 0; i < indent; i++)
      std::cout << " ";
    std::cout << "Initializer: " << std::endl;
    printAst(varDeclaration->initializer, indent + 2);
  }
}

void AstNode::printAst(AstNode *node, int indent) {
  if (!node)
    return;

  for (int i = 0; i < indent; i++)
    std::cout << " ";

  switch (node->type) {
  case AstNodeType::PROGRAM: {
    AstNode::Program *program = (AstNode::Program *)node->data;
    std::cout << "Program:" << std::endl;
    for (AstNode *stmt : program->statements)
      printAst(stmt, 1);
    break;
  }

  case AstNodeType::IDENTIFIER: {
    AstNode::Identifier *identifier = (AstNode::Identifier *)node->data;
    identifier->name.start =
        strtok(const_cast<char *>(identifier->name.start), ";");
    std::cout << "Identifier: " << identifier->name.start << std::endl;
    break;
  }
  case AstNodeType::BINARY: {
    AstNode::Binary *binary = (AstNode::Binary *)node->data;
    std::cout << "Binary-TK-Kind: " << binary->op << std::endl;
    printAst(binary->left, indent + 3);
    printAst(binary->right, indent + 3);
    break;
  }
  case AstNodeType::UNARY: {
    AstNode::Unary *unary = (AstNode::Unary *)node->data;
    std::cout << "Unary-TK-Kind: " << unary->op << std::endl;
    printAst(unary->right, indent + 2);
    break;
  }
  case AstNodeType::GROUPING: {
    AstNode::Grouping *grouping = (AstNode::Grouping *)node->data;
    std::cout << "Grouping: " << std::endl;
    printAst(grouping->expression, indent + 2);
    break;
  }

  case AstNodeType::NUMBER_LITERAL: {
    AstNode::NumberLiteral *numberLiteral =
        (AstNode::NumberLiteral *)node->data;
    std::cout << "NumberLiteral: " << numberLiteral->value << std::endl;
    break;
  }
  case AstNodeType::STRING_LITERAL: {
    AstNode::StringLiteral *stringLiteral =
        (AstNode::StringLiteral *)node->data;
    stringLiteral->value =
        stringLiteral->value.substr(1, stringLiteral->value.length() - 6);
    std::cout << "StringLiteral: " << stringLiteral->value << std::endl;
    break;
  }
  case AstNodeType::TRUE_LITERAL: {
    std::cout << "TrueLiteral" << std::endl;
    break;
  }
  case AstNodeType::FALSE_LITERAL: {
    std::cout << "FalseLiteral" << std::endl;
    break;
  }
  case AstNodeType::NIL_LITERAL: {
    std::cout << "NilLiteral" << std::endl;
    break;
  }

  case AstNodeType::TYPE: {
    AstNode::Type *type = (AstNode::Type *)node->data;
    std::cout << "Type-TK-Kind: " << type->type.kind << std::endl;
    break;
  }
  case AstNodeType::TYPE_STRUCT: {
    AstNode::TypeStruct *typeStruct = (AstNode::TypeStruct *)node->data;
    std::cout << "TypeStruct: " << std::endl;
    std::cout << "Name: " << typeStruct->name.start << std::endl;
    for (AstNode *field : typeStruct->fields)
      printAst(field, indent + 2);
    break;
  }

  case AstNodeType::EXPRESSION: {
    AstNode::Expression *expression = (AstNode::Expression *)node->data;
    std::cout << "Expression: " << std::endl;
    printAst(expression->expression, indent + 2);
    break;
  }
  case AstNodeType::VAR_DECLARATION: {
    AstNode::VarDeclaration *varDeclaration =
        (AstNode::VarDeclaration *)node->data;
    std::cout << "VarDeclaration: " << std::endl;
    printVarDeclaration(varDeclaration, indent + 2);
    break;
  }
  case AstNodeType::FUNCTION_DECLARATION: {
    AstNode::FunctionDeclaration *functionDeclaration =
        (AstNode::FunctionDeclaration *)node->data;
    std::cout << "FunctionDeclaration: " << std::endl;
    functionDeclaration->name.start =
        strtok(const_cast<char *>(functionDeclaration->name.start), "(");
    for (int i = 0; i < indent + 2; i++)
      std::cout << " ";
    std::cout << "Name: " << functionDeclaration->name.start << std::endl;
    
    // Find the parameters and their types
    if (functionDeclaration->parameters.size() > 0) {
      for (Lexer::Token parameter : functionDeclaration->parameters) {
        const char *type = nullptr;
        switch (functionDeclaration->paramType.front()->type) {
        case AstNodeType::TYPE: {
          AstNode::Type *typeNode =
              (AstNode::Type *)functionDeclaration->paramType.front()->data;
          type = typeNode->type.start;
          type = strtok(const_cast<char *>(type), ",");
          type = strtok(const_cast<char *>(type), ")");
          break;
        }
        default:
          break;
        }

        const char *name = strtok(const_cast<char *>(parameter.start), " ");
        name = strtok(const_cast<char *>(name), ":");
        for (int i = 0; i < indent + 2; i++)
          std::cout << " ";
        std::cout << "Parameter: " << std::endl;
        for (int i = 0; i < indent + 4; i++)
          std::cout << " ";
        std::cout << "Name: " << name << std::endl;
        for (int i = 0; i < indent + 4; i++)
          std::cout << " ";
        std::cout << "Type: " << type << std::endl;
      }
    }

    printAst(functionDeclaration->type, indent + 2);
    printAst(functionDeclaration->body, indent + 2);
    break;
  }
  case AstNodeType::BLOCK: {
    AstNode::Block *block = (AstNode::Block *)node->data;
    std::cout << "Block: " << std::endl;
    for (AstNode *stmt : block->statements)
      printAst(stmt, indent + 2);
    break;
  }
  case AstNodeType::PRINT: {
    AstNode::Print *print = (AstNode::Print *)node->data;
    std::cout << "Print: " << std::endl;
    printAst(print->expression, indent + 2);
    print->ident.start = strtok(const_cast<char *>(print->ident.start), ";");
    for (int i = 0; i < indent + 2; i++)
      std::cout << " ";
    std::cout << "Identifier: " << print->ident.start << std::endl;
    break;
  }
  case AstNodeType::EXIT: {
    AstNode::Exit *exit = (AstNode::Exit *)node->data;
    std::cout << "Exit: " << std::endl;
    printAst(exit->expression, indent + 2);
    break;
  }

  default:
    std::cout << "Unknown node type" << std::endl;
    break;
  }
}
