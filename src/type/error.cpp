#include <cstring>
#include <iostream>

#include "../../inc/colorize.hpp"
#include "../ast/ast.hpp"
#include "../common.hpp"
#include "type.hpp"

void Type::checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                          std::string name) {

  type = determineIfUpCast(type, returnType);

  std::cout << "type: " << type->type.start << std::endl;
  std::cout << "returnType: " << returnType->type.start << std::endl;

  // string compare the type and returnType
  if (strcmp(type->type.start, returnType->type.start) == 0)
    return;

  std::cout << termcolor::red << "Error: " << termcolor::reset
            << "Expected type '" << type->type.start << "' but got '"
            << returnType->type.start << "' in function '" << name << "'"
            << std::endl;
  Exit(ExitValue::INVALID_TYPE);
}
