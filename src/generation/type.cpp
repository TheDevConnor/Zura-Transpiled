#include <fstream>
#include <cstring>

#include "gen.hpp"

const char* Gen::findType(AstNode *node) {
  switch (node->type) {
  case AstNodeType::TYPE: {
    AstNode::Type *type = (AstNode::Type *)node->data;
    type->type.start = strtok(const_cast<char *>(type->type.start), ">");
    return type->type.start;
  }
  default:
    break;
  }

  return "Unknown";
}

void Gen::printTypeToFile(std::ofstream &file, const char* type) {
  switch (type[0])
  {
  case 'i':
    if (type[1] == '1') {
      file << "int16_t ";
    } else if (type[1] == '3') {
      file << "int32_t ";
    } else if (type[1] == '6') {
      file << "int64_t ";
    } else {
      file << "int ";
    }
    break;
  case 'u':
    file << "unsigned ";
    break;
  case 'f':
    file << "float ";
    break;
  case 's':
    file << "char* ";
    break;
  case 'c':
    file << "char ";
    break;
  case 'b':
    file << "bool ";
    break;
  default:
    break;
  }
}