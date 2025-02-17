#include "../ast/expr.hpp"
#include "../codegen/gen.hpp"
#include "parser.hpp"

#include <iostream>
#include <unordered_map>

/**
 * Parses an expression from the given parser and returns the resulting
 * expression node. This is the bread and butter of the Pratt Parser. It uses
 * the nud and led functions to parse the expression based on the binding power
 * of the current token.
 *
 * @param psr The parser object.
 * @param bp The binding power to use for parsing.
 * @return The parsed expression node.
 */
Node::Expr *Parser::parseExpr(PStruct *psr, BindingPower bp) {
  Node::Expr *left = nud(psr);

  while (getBP(psr->current(psr).kind) > bp) {
    left = led(psr, left, getBP(psr->current(psr).kind));
  }

  return left;
}

Node::Expr *Parser::primary(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  switch (psr->current(psr).kind) {
  case TokenKind::INT: {
    return new IntExpr(line, column, std::stoll(psr->advance(psr).value), codegen::getFileID(psr->current_file));
  }
  case TokenKind::FLOAT: {
    return new FloatExpr(line, column, std::stof(psr->advance(psr).value), codegen::getFileID(psr->current_file));
  }
  case TokenKind::IDENTIFIER: {
    return new IdentExpr(line, column, psr->advance(psr).value, nullptr, codegen::getFileID(psr->current_file));
  }
  case TokenKind::STRING: {
    return new StringExpr(line, column, psr->advance(psr).value, codegen::getFileID(psr->current_file));
  }
  case TokenKind::CHAR: {
    return new CharExpr(line, column, psr->advance(psr).value[1], codegen::getFileID(psr->current_file));
  }
  default:
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Could not parse primary expression!", "", "Parser Error",
                      node.current_file, lexer, psr->tks, true, false, false, false,
                      false, false);
    return nullptr;
  }
}

Node::Expr *Parser::group(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected L_Paran before a grouping expr!");
  Node::Expr *expr = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected R_Paran after a grouping expr!");

  return new GroupExpr(line, column, expr, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::unary(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr);
  Node::Expr *right = parseExpr(psr, defaultValue);

  return new UnaryExpr(line, column, right, op.value, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::_prefix(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr);
  Node::Expr *right = parseExpr(psr, defaultValue);

  return new PrefixExpr(line, column, right, op.value, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::alloc_expr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::ALLOC, "Expected 'ALLOC' keyword!");
  psr->expect(psr, TokenKind::LEFT_PAREN, "Expected L_Paran for an alloc expr!");

  Node::Expr *bytes = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected R_Paran to end an alloc expr!");
  return new AllocMemoryExpr(line, column, bytes, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::free_expr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::FREE, "Expected 'FREE' keyword!");
  psr->expect(psr, TokenKind::LEFT_PAREN, "Expected L_Paran for a free memory expression!");

  Node::Expr *whatToFree = parseExpr(psr, defaultValue); 
  if (psr->current(psr).kind == TokenKind::COMMA)
    psr->expect(psr, TokenKind::COMMA, "Expected a COMMA after the expression to free!");
  Node::Expr *bytesToFree = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected R_Paran to end a free memory expression!");

  return new FreeMemoryExpr(line, column, whatToFree, bytesToFree, codegen::getFileID(psr->current_file));
};

// update cast syntax to `@cast<type>(expr)`
Node::Expr *Parser::cast_expr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, CAST, "expected 'CAST' keyword!");

  Node::Expr *castee = nullptr;
  Node::Type *castee_type = nullptr;

  psr->expect(psr, TokenKind::LESS, "Expected a 'LESS' to start a cast!");
  castee_type = parseType(psr, defaultValue);
  psr->expect(psr, TokenKind::GREATER, "Expected GREATER to begin a cast!");

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_Paran to be able to cast an expression!");
  castee = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_Paran to end a cast expr!");

  return new CastExpr(line, column, castee, castee_type, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::sizeof_expr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(psr, SIZEOF, "Expected 'SIZEOF' keyword!");

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a sizeof expr!");
  Node::Expr *expr = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_Paran to end a sizeof expr!");

  return new SizeOfExpr(line, column, expr, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::memcpy_expr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(psr, MEMCPY, "Expected 'MEMCPY' keyword!");

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a memcpy expr!");
  Node::Expr *dest = parseExpr(psr, defaultValue);
  
  if (psr->current(psr).kind == TokenKind::COMMA)
    psr->expect(psr, TokenKind::COMMA,
                "Expected a COMMA after the destination in a memcpy expr!");

  Node::Expr *src = parseExpr(psr, defaultValue);
  
  if (psr->current(psr).kind == TokenKind::COMMA)
    psr->expect(psr, TokenKind::COMMA,
                "Expected a COMMA after the source in a memcpy expr!");

  Node::Expr *size = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_Paran to end a memcpy expr!");

  return new MemcpyExpr(line, column, dest, src, size, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::_postfix(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr);
  return new PostfixExpr(line, column, left, op.value, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::array(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_BRACKET,
              "Expected a L_Bracket to start an array expr!");
  std::vector<Node::Expr *> elements;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACKET) {
    if (psr->current(psr).kind == TokenKind::LEFT_BRACE) {
      elements.push_back(structExpr(psr));
    } else {
      elements.push_back(parseExpr(psr, defaultValue));
    }

    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after an element in an array expr!");
  }

  psr->expect(psr, TokenKind::RIGHT_BRACKET,
              "Expected a R_Bracket to end an array expr!");

  // check if the array has only one elem and that the elem is equal to 0
  // if its an array of bools or chars that will also seg
  if (elements.size() == 1 && static_cast<IntExpr*>(elements.at(0))->value == 0) {
    return new ArrayAutoFill(line, column, codegen::getFileID(psr->current_file));
  }
  return new ArrayExpr(line, column, nullptr, elements, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr);

  Node::Expr *right = parseExpr(psr, bp);

  return new BinaryExpr(line, column, left, right, op.value, codegen::getFileID(psr->current_file));
}

//  # change a variable in the array
// x[2] = 10; # x = [1, 2, 10, 4, 5]
//
// # pop a specific index from the array
// x[<-2]; # z = 3, x = [1, 2, 4, 5]
//
// # push a variable to the array
// x[->6]; # a = 6, x = [1, 2, 4, 5, 6]
//
// # push a variable to the array at a specific index
// x[->7 @ 2]; # x = [1, 2, 7, 4, 5, 6]
//
// # pop the last element from the array
// x[<-]; # b = 6, x = [1, 2, 7, 4, 5]

Node::Expr *Parser::index(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_BRACKET,
              "Expected a L_Bracket to start an index expr!");

  Node::Expr *index = nullptr;
  switch (psr->current(psr).kind) {
    // This is a pop operation
  case TokenKind::LEFT_ARROW: {
    psr->expect(psr, TokenKind::LEFT_ARROW,
                "Expected a L_Arrow to pop an index from an array!");

    // check if the next token is a right bracket to pop the last element
    if (psr->current(psr).kind == TokenKind::RIGHT_BRACKET) {
      psr->advance(psr);
      return new PopExpr(line, column, left, nullptr, codegen::getFileID(psr->current_file));
    }

    index = parseExpr(psr, defaultValue);

    psr->expect(psr, TokenKind::RIGHT_BRACKET,
                "Expected a R_Bracket to end an index expr!");
    return new PopExpr(line, column, left, index, codegen::getFileID(psr->current_file));
  }
  case TokenKind::RIGHT_ARROW: {
    psr->expect(psr, TokenKind::RIGHT_ARROW,
                "Expected a R_Arrow to push an index to an array!");

    index = parseExpr(psr, defaultValue);

    if (psr->current(psr).kind == TokenKind::AT) {
      psr->expect(psr, TokenKind::AT,
                  "Expected an AT to specify an index for a push operation!");
      Node::Expr *push_index = parseExpr(psr, defaultValue);
      psr->expect(psr, TokenKind::RIGHT_BRACKET,
                  "Expected a R_Bracket to end an index expr!");
      return new PushExpr(line, column, left, index, push_index, codegen::getFileID(psr->current_file));
    }

    psr->expect(psr, TokenKind::RIGHT_BRACKET,
                "Expected a R_Bracket to end an index expr!");
    return new PushExpr(line, column, left, index, nullptr, codegen::getFileID(psr->current_file));
  }
  default:
    // This is a normal index
    index = parseExpr(psr, defaultValue);
    break;
  }

  psr->expect(psr, TokenKind::RIGHT_BRACKET,
              "Expected a R_Bracket to end an index expr!");

  return new IndexExpr(line, column, left, index, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::assign(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr);
  Node::Expr *right = parseExpr(psr, defaultValue);

  return new AssignmentExpr(line, column, left, op.value, right, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::parse_call(PStruct *psr, Node::Expr *left,
                               BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a call expr!");
  std::vector<Node::Expr *> args;

  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, defaultValue));
    
    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after an argument in a call expr!");
  }

  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_Paren to end a call expr!");

  return new CallExpr(line, column, left, args, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::_ternary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::QUESTION,
              "Expected a '?' to define the true value of an ternary expr!");
  Node::Expr *true_expr = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::COLON,
              "Expected a ':' to define the false value of a ternary Expr!");
  Node::Expr *false_expr = parseExpr(psr, defaultValue);

  return new TernaryExpr(line, column, left, true_expr, false_expr, codegen::getFileID(psr->current_file));
}


// @call<NativeFunctionName>(fnuctionArgs);
// differnt than functionName();
Node::Expr *Parser::externalCall(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::CALL,
              "Expected a CALL keyword to start a call stmt");
  
  psr->expect(psr, TokenKind::LESS,
              "Expected a LESS to start call function name");
  
  std::string funcName = psr->expect(psr, TokenKind::IDENTIFIER,
              "Expected an IDENTIFIER as a function name in a call stmt")
            .value;

  psr->expect(psr, TokenKind::GREATER,
              "Expected a GREATER to end call function name");
  
  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a LEFT_PAREN to start call function arguments");
  
  std::vector<Node::Expr *> args;
  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, BindingPower::defaultValue));
    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after an arguement in a call stmt");
  }

  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a RIGHT_PAREN to end call function arguments");

  return new ExternalCall(line, column, funcName, args, codegen::getFileID(psr->current_file));
};

Node::Expr *Parser::_member(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr); // This should be a DOT
  Node::Expr *right = parseExpr(psr, member);
  return new MemberExpr(line, column, left, right, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::resolution(PStruct *psr, Node::Expr *left,
                               BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance(psr);
  Node::Expr *right = parseExpr(psr, member);

  return new ResolutionExpr(line, column, left, right, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::bool_expr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  bool res = (psr->advance(psr).value == "true") ? true : false;

  return new BoolExpr(line, column, res, codegen::getFileID(psr->current_file));
}

// {a: 1, b: 2, c: 3}
Node::Expr *Parser::structExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(psr, TokenKind::LEFT_BRACE,
              "Expected a L_Brace to start a struct expr!");

  std::unordered_map<std::string, Node::Expr *> elements;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
    std::string key = psr->expect(psr, TokenKind::IDENTIFIER,
                "Expected an IDENTIFIER as a key in a struct expr!")
                .value;
    psr->expect(psr, TokenKind::COLON,
                "Expected a COLON after a key in a struct expr!");
    Node::Expr *value = parseExpr(psr, defaultValue);
    elements[key] = value;

    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after a key-value pair in a struct expr!");
  }

  psr->expect(psr, TokenKind::RIGHT_BRACE,
              "Expected a R_Brace to end a struct expr!");

  return new StructExpr(line, column, elements, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::address(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  Lexer::Token op = psr->advance(psr);

  // Expect an rhs expression
  Node::Expr *expr = parseExpr(psr, defaultValue);

  return new AddressExpr(line, column, expr, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::null_type(PStruct *psr) {
  psr->advance(psr);
  return new NullExpr(psr->tks[psr->pos].line, psr->tks[psr->pos].column, codegen::getFileID(psr->current_file));
};