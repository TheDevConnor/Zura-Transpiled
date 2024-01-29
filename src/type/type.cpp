#include "../ast/ast.hpp"
#include "../visitor/visit.hpp"
#include "type.hpp"

void TypeClass::typeCheck(const AstNode &node) {
  AstVisitor visitor;

  visitor.typeCheckVisit(node);
}
