#include <cstring>
#include <fstream>
#include <iostream>

#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "gen.hpp"

void Gen::functionDeclaration(std::ofstream &file, AstNode* node) {
  AstNode::FunctionDeclaration *fun =
          (AstNode::FunctionDeclaration*)node->data;

  const char* type = findType(fun->type);
  printTypeToFile(file, type);

  const char* name = fun->name.start;
  name = strtok(const_cast<char *>(name), "(");
  file << name << "(";

  if (fun->parameters.size() > 0) {
    for (Lexer::Token parameter : fun->parameters) {
      const char* paramType;
      switch (fun->paramType.front()->type) {
      case AstNodeType::TYPE: {
        AstNode::Type *typeNode =
            (AstNode::Type *)fun->paramType.front()->data;
        paramType = typeNode->type.start;
        paramType = strtok(const_cast<char *>(paramType), ",");
        paramType = strtok(const_cast<char *>(paramType), ")");
        break;
      }
      default: break;
      }
      fun->paramType.erase(fun->paramType.begin());

      const char* paramName = strtok(const_cast<char *>(parameter.start), " ");
      paramName = strtok(const_cast<char *>(paramName), ":");
      printTypeToFile(file, paramType);
      file << paramName;
      if (fun->parameters.size() > 1) file << ", ";
    }
    file.seekp(-2, std::ios_base::end);
  }

  file << ") {\n";

  blockStmt(file, fun->body);

  file << "}\n\n";
}
