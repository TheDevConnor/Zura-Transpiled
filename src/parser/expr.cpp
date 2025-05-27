#include "../ast/expr.hpp"

#include <unordered_map>

#include "../codegen/gen.hpp"
#include "../helper/error/error.hpp"
#include "parser.hpp"

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

  while (getBP(psr->current().kind) > bp) {
    left = led(psr, left, getBP(psr->current().kind));
  }

  return left;
}

Node::Expr *Parser::primary(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  switch (psr->current().kind) {
  case TokenKind::INT: {
    return new IntExpr(line, column, std::stoll(psr->advance().value),
                       codegen::getFileID(psr->current_file));
  }
  case TokenKind::FLOAT: {
    return new FloatExpr(line, column, psr->advance().value,
                         codegen::getFileID(psr->current_file));
  }
  case TokenKind::IDENTIFIER: {
    return new IdentExpr(line, column, psr->advance().value, nullptr,
                         codegen::getFileID(psr->current_file));
  }
  case TokenKind::STRING: {
    return new StringExpr(line, column, psr->advance().value,
                          codegen::getFileID(psr->current_file));
  }
  case TokenKind::CHAR: {
    return new CharExpr(line, column, psr->advance().value[1],
                        codegen::getFileID(psr->current_file));
  }
  default:
    std::string msg =
        "Expected a primary expression, but got: " + psr->current().value;
    Error::handle_error("Parser", psr->current_file, msg, psr->tks, line,
                        column);
    return nullptr;
  }
}

Node::Expr *Parser::group(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::LEFT_PAREN,
              "Expected L_Paran before a grouping expr!");
  Node::Expr *expr = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected R_Paran after a grouping expr!");

  return new GroupExpr(line, column, expr,
                       codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::unary(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance();
  Node::Expr *right = parseExpr(psr, postfix);
  if (op.value == "-" && right->kind == ND_INT) {
    return new IntExpr(line, column, -(static_cast<IntExpr *>(right)->value),
                       codegen::getFileID(psr->current_file));
  }

  return new UnaryExpr(line, column, right, op.value,
                       codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::_prefix(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance();
  Node::Expr *right = parseExpr(psr, defaultValue);

  return new PrefixExpr(line, column, right, op.value,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::allocExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::ALLOC, "Expected 'ALLOC' keyword!");
  psr->expect(TokenKind::LEFT_PAREN, "Expected L_Paran for an alloc expr!");

  Node::Expr *bytes = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN, "Expected R_Paran to end an alloc expr!");
  return new AllocMemoryExpr(line, column, bytes,
                             codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::freeExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::FREE, "Expected 'FREE' keyword!");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected L_Paran for a free memory expression!");

  Node::Expr *whatToFree = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the expression to free!");
  Node::Expr *bytesToFree = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected R_Paran to end a free memory expression!");

  return new FreeMemoryExpr(line, column, whatToFree, bytesToFree,
                            codegen::getFileID(psr->current_file));
};

// update cast syntax to `@cast<type>(expr)`
Node::Expr *Parser::castExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->advance(); // advance past the @cast

  Node::Expr *castee = nullptr;
  Node::Type *castee_type = nullptr;

  psr->expect(TokenKind::LESS, "Expected a 'LESS' to start a cast!");
  castee_type = parseType(psr);
  psr->expect(TokenKind::GREATER, "Expected GREATER to begin a cast!");

  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paran to be able to cast an expression!");
  castee = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN, "Expected a R_Paran to end a cast expr!");

  return new CastExpr(line, column, castee, castee_type,
                      codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::sizeofExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(SIZEOF, "Expected 'SIZEOF' keyword!");

  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a sizeof expr!");
  Node::Expr *expr = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_Paran to end a sizeof expr!");

  return new SizeOfExpr(line, column, expr,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::memcpyExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(MEMCPY, "Expected 'MEMCPY' keyword!");

  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a memcpy expr!");
  Node::Expr *dest = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the destination in a memcpy expr!");
  Node::Expr *src = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the source in a memcpy expr!");

  Node::Expr *size = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_Paran to end a memcpy expr!");

  return new MemcpyExpr(line, column, dest, src, size,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::_postfix(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp;

  Lexer::Token op = psr->advance();
  return new PostfixExpr(line, column, left, op.value,
                         codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::array(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::LEFT_BRACKET,
              "Expected a L_Bracket to start an array expr!");
  std::vector<Node::Expr *> elements;

  while (psr->current().kind != TokenKind::RIGHT_BRACKET) {
    if (psr->current().kind == TokenKind::LEFT_BRACE) {
      elements.push_back(structExpr(psr));
    } else {
      elements.push_back(parseExpr(psr, BindingPower::_primary));
    }
    if (psr->current().kind == TokenKind::RIGHT_BRACKET)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after an element in an array expr!");
  }
  psr->expect(TokenKind::RIGHT_BRACKET,
              "Expected a R_Bracket to end an array expr!");

  // check if the array has only one elem and that the elem is equal to 0
  // if its an array of bools or chars that will also seg
  if (elements.size() == 1 &&
      static_cast<IntExpr *>(elements.at(0))->value == 0) {
    return new ArrayAutoFill(line, column,
                             codegen::getFileID(psr->current_file));
  }
  return new ArrayExpr(line, column, nullptr, elements,
                       codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Lexer::Token op = psr->advance();

  Node::Expr *right = parseExpr(psr, bp);

  return new BinaryExpr(line, column, left, right, op.value,
                        codegen::getFileID(psr->current_file));
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

// TODO: Simplify this function
Node::Expr *Parser::index(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  psr->expect(TokenKind::LEFT_BRACKET,
              "Expected a L_Bracket to start an index expr!");

  Node::Expr *index = nullptr;
  switch (psr->current().kind) {
    // This is a pop operation
  case TokenKind::LEFT_ARROW: {
    psr->expect(TokenKind::LEFT_ARROW,
                "Expected a L_Arrow to pop an index from an array!");

    // check if the next token is a right bracket to pop the last element
    if (psr->current().kind == TokenKind::RIGHT_BRACKET) {
      psr->advance();
      return new PopExpr(line, column, left, nullptr,
                         codegen::getFileID(psr->current_file));
    }

    index = parseExpr(psr, defaultValue);

    psr->expect(TokenKind::RIGHT_BRACKET,
                "Expected a R_Bracket to end an index expr!");
    return new PopExpr(line, column, left, index,
                       codegen::getFileID(psr->current_file));
  }
  case TokenKind::RIGHT_ARROW: {
    psr->expect(TokenKind::RIGHT_ARROW,
                "Expected a R_Arrow to push an index to an array!");

    index = parseExpr(psr, defaultValue);

    if (psr->current().kind == TokenKind::AT) {
      psr->expect(TokenKind::AT,
                  "Expected an AT to specify an index for a push operation!");
      Node::Expr *push_index = parseExpr(psr, defaultValue);
      psr->expect(TokenKind::RIGHT_BRACKET,
                  "Expected a R_Bracket to end an index expr!");
      return new PushExpr(line, column, left, index, push_index,
                          codegen::getFileID(psr->current_file));
    }

    psr->expect(TokenKind::RIGHT_BRACKET,
                "Expected a R_Bracket to end an index expr!");
    return new PushExpr(line, column, left, index, nullptr,
                        codegen::getFileID(psr->current_file));
  }
  default:
    // This is a normal index
    index = parseExpr(psr, defaultValue);
    break;
  }

  psr->expect(TokenKind::RIGHT_BRACKET,
              "Expected a R_Bracket to end an index expr!");
  return new IndexExpr(line, column, left, index,
                       codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::assign(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  Lexer::Token op = psr->advance();
  Node::Expr *right = parseExpr(psr, defaultValue);

  return new AssignmentExpr(line, column, left, op.value, right,
                            codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::parse_call(PStruct *psr, Node::Expr *left,
                               BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a call expr!");
  std::vector<Node::Expr *> args;

  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, defaultValue));

    if (psr->current().kind == TokenKind::RIGHT_PAREN)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after an argument in a call expr!");
  }

  psr->expect(TokenKind::RIGHT_PAREN, "Expected a R_Paren to end a call expr!");
  return new CallExpr(line, column, left, args,
                      codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::_ternary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  psr->expect(TokenKind::QUESTION,
              "Expected a '?' to define the true value of an ternary expr!");
  Node::Expr *true_expr = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COLON,
              "Expected a ':' to define the false value of a ternary Expr!");
  Node::Expr *false_expr = parseExpr(psr, defaultValue);

  return new TernaryExpr(line, column, left, true_expr, false_expr,
                         codegen::getFileID(psr->current_file));
}

// @call<NativeFunctionName>(fnuctionArgs);
// differnt than functionName();
Node::Expr *Parser::externalCall(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::CALL, "Expected a CALL keyword to start a call stmt");
  psr->expect(TokenKind::LESS, "Expected a LESS to start call function name");

  std::string funcName =
      psr->expect(TokenKind::IDENTIFIER,
                  "Expected an IDENTIFIER as a function name in a call stmt")
          .value;

  psr->expect(TokenKind::GREATER,
              "Expected a GREATER to end call function name");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a LEFT_PAREN to start call function arguments");

  std::vector<Node::Expr *> args;
  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, BindingPower::defaultValue));
    if (psr->current().kind == TokenKind::RIGHT_PAREN)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after an arguement in a call stmt");
  }

  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a RIGHT_PAREN to end call function arguments");
  return new ExternalCall(line, column, funcName, args,
                          codegen::getFileID(psr->current_file));
};

Node::Expr *Parser::_member(PStruct *psr, Node::Expr *left, BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  Lexer::Token op = psr->advance(); // This should be a DOT
  Node::Expr *right = parseExpr(psr, member);
  return new MemberExpr(line, column, left, right,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::resolution(PStruct *psr, Node::Expr *left,
                               BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  Lexer::Token op = psr->advance();
  Node::Expr *right = parseExpr(psr, member);

  return new ResolutionExpr(line, column, left, right,
                            codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::boolExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  bool res = (psr->advance().value == "true") ? true : false;

  return new BoolExpr(line, column, res, codegen::getFileID(psr->current_file));
}

// {a: 1, b: 2, c: 3}
Node::Expr *Parser::structExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(TokenKind::LEFT_BRACE,
              "Expected a L_Brace to start a struct expr!");

  std::unordered_map<std::string, Node::Expr *> elements;
  while (psr->current().kind != TokenKind::RIGHT_BRACE) {
    std::string key =
        psr->expect(TokenKind::IDENTIFIER,
                    "Expected an IDENTIFIER as a key in a struct expr!")
            .value;
    psr->expect(TokenKind::COLON,
                "Expected a COLON after a key in a struct expr!");
    Node::Expr *value = parseExpr(psr, defaultValue);
    elements[key] = value;

    if (psr->current().kind == TokenKind::RIGHT_BRACE)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after a key-value pair in a struct expr!");
  }

  psr->expect(TokenKind::RIGHT_BRACE,
              "Expected a R_Brace to end a struct expr!");
  return new StructExpr(line, column, elements,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::address(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  Lexer::Token op = psr->advance();

  // Expect an rhs expression
  Node::Expr *expr = parseExpr(psr, defaultValue);
  return new AddressExpr(line, column, expr,
                         codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::dereference(PStruct *psr, Node::Expr *left,
                                BindingPower bp) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)bp; // mark it as unused

  Lexer::Token op = psr->advance();
  return new DereferenceExpr(line, column, left,
                             codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::nullType(PStruct *psr) {
  psr->advance();
  return new NullExpr(psr->tks[psr->pos].line, psr->tks[psr->pos].column,
                      codegen::getFileID(psr->current_file));
};

Node::Expr *Parser::openExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::OPEN,
              "Expected an OPEN keyword to start an open expr!");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paren to start an open expr!");

  Node::Expr *filePath = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the file path in an open expr!");

  Node::Expr *canRead = nullptr;
  Node::Expr *canWrite = nullptr;
  Node::Expr *canCreate = nullptr;

  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    if (canRead == nullptr) {
      canRead = parseExpr(psr, defaultValue);
    } else if (canWrite == nullptr) {
      canWrite = parseExpr(psr, defaultValue);
    } else if (canCreate == nullptr) {
      canCreate = parseExpr(psr, defaultValue);
    } else {
      std::string msg = "Expected a maximum of 3 arguments for an open expr!";
      Error::handle_error("Parser", psr->current_file, msg, psr->tks, line,
                          column);
    }
    if (psr->current().kind == TokenKind::RIGHT_PAREN)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA between the open expr arguments!");
  }

  // this time there MUST be a closing parenthesis
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_Paren to end an open expr!");

  // Return!
  if (canRead == nullptr)
    canRead =
        new BoolExpr(line, column, true, codegen::getFileID(psr->current_file));
  if (canWrite == nullptr)
    canWrite =
        new BoolExpr(line, column, true, codegen::getFileID(psr->current_file));
  if (canCreate == nullptr)
    canCreate =
        new BoolExpr(line, column, true, codegen::getFileID(psr->current_file));
  return new OpenExpr(line, column, filePath, canRead, canWrite, canCreate,
                      codegen::getFileID(psr->current_file));
};

Node::Expr *Parser::getArgc(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::GETARGC, "Expected a GETARGC keyword to start.");
  psr->expect(TokenKind::LEFT_PAREN, "Expected a L_PAREN to start a getArgc");
  psr->expect(TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end a getArgc");

  return new GetArgcExpr(line, column, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::getArgv(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::GETARGV, "Expected a GETARGV keyword to start.");
  psr->expect(TokenKind::LEFT_PAREN, "Expected a L_PAREN to start a getArgv");
  psr->expect(TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end a getArgv");

  return new GetArgvExpr(line, column, codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::strcmp(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::STRCMP, "Expected a STRCMP keyword to start.");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected LEFT_PAREN after the STRCMP keyword.");
  Node::Expr *v1 = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA, "Expected a COMMA between the two values.");
  Node::Expr *v2 = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN, "Expected RIGHT_PAREN to end the expr");

  return new StrCmp(line, column, v1, v2,
                    codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::socketExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::SOCKET,
              "Expected a SOCKET keyword to start a socket expr!");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paren to start a socket expr!");

  Node::Expr *domain = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the domain in a socket expr!");

  Node::Expr *type = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the type in a socket expr!");

  Node::Expr *protocol = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_Paren to end a socket expr!");

  return new SocketExpr(line, column, domain, type, protocol,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::bindExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::BIND, "Expected a BIND keyword to start a bind expr!");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paren to start a bind expr!");

  Node::Expr *socket = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the socket in a bind expr!");

  Node::Expr *address = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the address in a bind expr!");

  Node::Expr *port = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN, "Expected a R_Paren to end a bind expr!");

  return new BindExpr(line, column, socket, address, port,
                      codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::listenExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::LISTEN,
              "Expected a LISTEN keyword to start a listen expr!");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paren to start a listen expr!");

  Node::Expr *socket = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the socket in a listen expr!");
  Node::Expr *backlog = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_Paren to end a listen expr!");

  return new ListenExpr(line, column, socket, backlog,
                        codegen::getFileID(psr->current_file));
}

Node::Expr *Parser::acceptExpr(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::ACCEPT,
              "Expected an ACCEPT keyword to start an accept expr!");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_Paren to start an accept expr!");

  Node::Expr *socket = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the socket in an accept expr!");
  Node::Expr *address = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the address in an accept expr!");
  Node::Expr *port = parseExpr(psr, defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_Paren to end an accept expr!");

  return new AcceptExpr(line, column, socket, address, port,
                        codegen::getFileID(psr->current_file));
}
