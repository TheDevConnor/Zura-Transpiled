#include <cstring>
#include <iostream>

#include "../../inc/colorize.hpp"
#include "../ast/ast.hpp"
#include "../common.hpp"
#include "type.hpp"

void Type::checkForErrors(AstNode::Type *type, AstNode::Type *returnType,
                          std::string name) {
  if (strcmp(type->type.start, returnType->type.start) == 0) {
    std::cout << termcolor::green << "Success: " << termcolor::reset
              << "Expected type '" << type->type.start << "' and got '"
              << returnType->type.start << "' in function '" << name << "'"
              << std::endl;
    return;
  }


  std::cout << termcolor::red << "Error: " << termcolor::reset
            << "Expected type '" << type->type.start << "' but got '"
            << returnType->type.start << "' in function '" << name << "'"
            << std::endl;
  Exit(ExitValue::INVALID_TYPE);
}
