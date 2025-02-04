#pragma once

#include "../lexer/lexer.hpp"

#include <iostream>
#include <string>
#include <vector>

enum NodeKind {
  // Expressions
  ND_INT, // 123
  ND_FLOAT, // 12.3
  ND_IDENT, // x
  ND_STRING, // "hello"
  ND_CHAR, // 'c'
  ND_BINARY, // 12 + 34
  ND_UNARY, // !true
  ND_PREFIX, // ++x
  ND_POSTFIX, // x++
  ND_GROUP, // (x + 12)
  ND_ARRAY, // [x, 12]
  ND_INDEX, // x[12]
  ND_ARRAY_AUTO_FILL, // x: [5]int = {0}
  ND_POP, // x.pop
  ND_PUSH, // x.push
  ND_CALL, // x()
  ND_TEMPLATE_CALL, // x<12.3>()
  ND_ASSIGN, // x = 12
  ND_TERNARY, // x == 12 ? .. : ..
  ND_MEMBER, // x.y
  ND_RESOLUTION, // x::y
  ND_BOOL, // true
  ND_CAST, // @cast<int>(12.3)
  ND_EXTERNAL_CALL, // @call<LinkedFunction>(12);
  ND_STRUCT, // have x: struct = { val = 12 };
  ND_ADDRESS, // &x

  // Statements
  ND_EXPR_STMT, // x = 12
  ND_VAR_STMT, // have x := 12
  ND_CONST_STMT, // const x := 12
  ND_BLOCK_STMT, // { x = 12 }
  ND_FN_STMT, // const x := fn () int {}
  ND_PROGRAM, // { .. x = 12; fn (); }
  ND_RETURN_STMT, // return 12
  ND_IF_STMT, // if (x) { return 12 } 
  ND_STRUCT_STMT, // struct x { val = 12 }
  ND_WHILE_STMT, // loop (x) { ... }
  ND_FOR_STMT, // loop (x = 1, x < 10) { ... }
  ND_PRINT_STMT, // dis(x)
  ND_ENUM_STMT, // enum x {}
  ND_IMPORT_STMT, // import "x"
  ND_TEMPLATE_STMT, // template <T> fn () T {}
  ND_BREAK_STMT,  // break
  ND_CONTINUE_STMT, // continue
  ND_LINK_STMT, // link "x"
  ND_EXTERN_STMT, // extern "C"
  ND_MATCH_STMT, // match (x) { case 123 => { ... } }
  ND_INPUT_STMT, // @input

  // Types
  ND_SYMBOL_TYPE, // int
  ND_ARRAY_TYPE, // []int
  ND_POINTER_TYPE, // *int
  ND_CALLABLE_TYPE, // Idk honestly
  ND_FUNCTION_TYPE, // fn (int) int
  ND_TEMPLATE_STRUCT_TYPE, // struct <T> { val = T }
  ND_FUNCTION_TYPE_PARAM, // fn (int) int
  ND_NULL, // nil
};

class Node {
public:
  // Store the list of tks generated by the parser
  // so that we can generate the line from the line and
  // pos stored on the ast nodes; for error reporting
  std::vector<Lexer::Token> tks;
  // Store the current file name
  std::string current_file;
  struct Type {
    NodeKind kind; // Pointer, Array, Symbol
    virtual void debug(int ident = 0) const = 0;
    virtual ~Type() = default;
  };
  
  struct Expr {
    NodeKind kind;
    int file_id;
    Type *asmType;
    virtual void debug(int ident = 0) const = 0;
    virtual ~Expr() = default;
  };

  struct Stmt {
    NodeKind kind;
    int file_id;
    virtual void debug(int ident = 0) const = 0;
    virtual ~Stmt() = default;
  };

  static void printIndent(int ident) {
    for (int i = 0; i < ident; i++) {
      std::cout << "    ";
    }
  }
};

// This is probably a really bad idea,
// but it works so i am not touching it
inline Node node;