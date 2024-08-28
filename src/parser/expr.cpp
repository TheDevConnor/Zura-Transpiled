#include "../ast/expr.hpp"
#include "parser.hpp"

#include <iostream>

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
  auto *left = nud(psr);

  while (getBP(psr->current(psr).kind) > bp) {
    left = led(psr, left, getBP(psr->current(psr).kind));
  }

  return left;
}

Node::Expr *Parser::primary(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  switch (psr->current(psr).kind) {
  case TokenKind::NUMBER: {
    return new NumberExpr(line, column, std::stod(psr->advance(psr).value));
  }
  case TokenKind::IDENTIFIER: {
    return new IdentExpr(line, column, psr->advance(psr).value);
  }
  case TokenKind::STRING: {
    return new StringExpr(line, column, psr->advance(psr).value);
  }
  default:
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Could not parse primary expression!", "", "Parser Error",
                      "main.zu", lexer, psr->tks, true, false, false, false,
                      false, false);
    return nullptr;
  }
}

Node::Expr *Parser::group(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected L_Paran before a grouping expr!");
  auto *expr = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected R_Paran after a grouping expr!");
  return new GroupExpr(line, column, expr);
}

Node::Expr *Parser::unary(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto op = psr->advance(psr);
  auto *right = parseExpr(psr, defaultValue);

  return new UnaryExpr(line, column, right, op.value);
}

Node::Expr *Parser::_prefix(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto op = psr->advance(psr);
  auto *right = parseExpr(psr, defaultValue);

  return new PrefixExpr(line, column, right, op.value);
}

Node::Expr *Parser::array(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_BRACKET,
              "Expected a L_Bracket to start an array expr!");
  std::vector<Node::Expr *> elements;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACKET) {
    elements.push_back(parseExpr(psr, defaultValue));
    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after an element in an array expr!");
  }

  psr->expect(psr, TokenKind::RIGHT_BRACKET,
              "Expected a R_Bracket to end an array expr!");

  return new ArrayExpr(line, column, nullptr, elements); 
}

Node::Expr *Parser::binary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto op = psr->advance(psr);
  auto *right = parseExpr(psr, bp);

  return new BinaryExpr(line, column, left, right, op.value);
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
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

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
      return new PopExpr(line, column, left, nullptr); 
    }

    index = parseExpr(psr, defaultValue);
    
    psr->expect(psr, TokenKind::RIGHT_BRACKET,
                "Expected a R_Bracket to end an index expr!");
    return new PopExpr(line, column, left, index);
  }
  case TokenKind::RIGHT_ARROW: {
    psr->expect(psr, TokenKind::RIGHT_ARROW,
                "Expected a R_Arrow to push an index to an array!");

    index = parseExpr(psr, defaultValue);

    if (psr->current(psr).kind == TokenKind::AT) {
      psr->expect(psr, TokenKind::AT,
                  "Expected an AT to specify an index for a push operation!");
      auto *push_index = parseExpr(psr, defaultValue);
      psr->expect(psr, TokenKind::RIGHT_BRACKET,
                  "Expected a R_Bracket to end an index expr!");
      return new PushExpr(line, column, left, index, push_index);
    }
    
    psr->expect(psr, TokenKind::RIGHT_BRACKET,
                "Expected a R_Bracket to end an index expr!");
    return new PushExpr(line, column, left, index, nullptr); 
  }
  default:
    // This is a normal index
    index = parseExpr(psr, defaultValue);
    break;
  }

  psr->expect(psr, TokenKind::RIGHT_BRACKET,
              "Expected a R_Bracket to end an index expr!");

  return new IndexExpr(line, column, left, index);
}

Node::Expr *Parser::assign(PStruct *psr, Node::Expr *left, BindingPower bp) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto op = psr->advance(psr);
  auto *right = parseExpr(psr, defaultValue);

  return new AssignmentExpr(line, column, left, op.value, right);
}

Node::Expr *Parser::parse_call(PStruct *psr, Node::Expr *left,
                               BindingPower bp) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_Paran to start a call expr!");
  std::vector<Node::Expr *> args;

  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, defaultValue));
    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after an arguement!");
  }

  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_Paren to end a call expr!");

  return new CallExpr(line, column, left, args);
}

Node::Expr *Parser::_ternary(PStruct *psr, Node::Expr *left, BindingPower bp) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::QUESTION,
              "Expected a '?' to define the true value of an ternary expr!");
  auto *true_expr = parseExpr(psr, defaultValue);
  psr->expect(psr, TokenKind::COLON,
              "Expected a ':' to define the false value of a ternary Expr!");
  auto *false_expr = parseExpr(psr, defaultValue);

  return new TernaryExpr(line, column, left, true_expr, false_expr);
}

Node::Expr *Parser::_member(PStruct *psr, Node::Expr *left, BindingPower bp) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto op = psr->advance(psr); // This should be a DOT
  auto *right = parseExpr(psr, member);
  return new MemberExpr(line, column, left, right);
}

Node::Expr *Parser::resolution(PStruct *psr, Node::Expr *left,
                               BindingPower bp) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto op = psr->advance(psr);
  auto *right = parseExpr(psr, member);

  return new ResolutionExpr(line, column, left, right);
}

Node::Expr *Parser::bool_expr(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto res = (psr->advance(psr).value == "true") ? true : false;

  return new BoolExpr(line, column, res);
}