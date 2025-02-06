// Compiler-time optimizations
// For example, in C, binary expressions on literals are automatically calculated
// and an unsigned-int division of 2 is automatically converted to a right shift, along with the other powers.
#include <set>

#include "../../ast/expr.hpp"
#include "../../ast/stmt.hpp"
#include "../../ast/types.hpp"

class CompileOptimizer {
public:

  static Node::Expr *optimizeExpr(Node::Expr *expr);
  static Node::Expr *optimizeUnary(UnaryExpr *expr);
  static Node::Expr *optimizeBinary(BinaryExpr *expr);
  
  static Node::Stmt *optimizeIfStmt(IfStmt *stmt);
  static Node::Stmt *optimizeStmt(Node::Stmt *stmt);
};

// Binary operations that can be optimized
static inline std::set<std::string> intOperations = {
  {"+"}, {"-"}, {"*"}, {"/"}, {"%"}, {"&"}, {"|"}, {"^"}, {"<<"}, {">>"}
};
static inline std::set<std::string> boolOperations = {
  {"&&"}, {"||"}, {"=="}, {"!="}, {"<"}, {">"}, {"<="}, {">="}
};