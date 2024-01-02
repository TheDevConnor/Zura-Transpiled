#include <cstring>
#include <iostream>

#include "../../inc/colorize.hpp"
#include "../ast/ast.hpp"
#include "../common.hpp"
#include "type.hpp"

void Type::checkCastOfStr(AstNode::Type *type, AstNode::Type *returnType,
                          std::string name) {
    if (strcmp(type->type.start, "str") == 0 &&
    strcmp(returnType->type.start, "str") != 0) {
    std::cout << termcolor::red << "Error: " << termcolor::reset
              << "Cannot cast '" << type->type.start << "' to '"
              << returnType->type.start << "' in declartion of '" << name << "'"
              << std::endl;
    Exit(ExitValue::INVALID_TYPE);
  }
}

void Type::checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                          std::string name) {
  checkCastOfStr(type, returnType, name);
  type = determineIfUpCast(type, returnType);
  type = determineIfDownCast(type, returnType);

  // string compare the type and returnType
  if (strcmp(type->type.start, returnType->type.start) == 0)
    return;

  std::cout << termcolor::red << "Error: " << termcolor::reset
            << "Expected type '" << type->type.start << "' but got '"
            << returnType->type.start << "' in declartion of '" << name << "'"
            << std::endl;
  Exit(ExitValue::INVALID_TYPE);
}
