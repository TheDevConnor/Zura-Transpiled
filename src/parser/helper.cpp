#include "parser.hpp"

bool Parser::PStruct::hadTokens(PStruct *psr) {
  return psr->pos < psr->tks.size();
}

Lexer::Token Parser::PStruct::current(PStruct *psr) {
  if (psr->pos >= psr->tks.size()) {
    return psr->tks[psr->tks.size() -
                    1]; // Return the last EOF token. This might fall all the
                        // way down into an "expect" and create a NICE error
                        // instead of segfaulting.
  }
  return psr->tks[psr->pos];
}

Lexer::Token Parser::PStruct::advance(PStruct *psr) {
  Lexer::Token tk = current(psr);
  psr->pos++;
  return tk;
}

Lexer::Token Parser::PStruct::peek(PStruct *psr, int offset) {
  if (psr->pos + offset >= psr->tks.size()) {
    return psr->tks[psr->tks.size() -
                    1]; // Return the last EOF token. This might fall all the
                        // way down into an "expect" and create a NICE error
                        // instead of segfaulting.
  }
  return psr->tks[psr->pos + offset];
}

Lexer::Token Parser::PStruct::expect(PStruct *psr, TokenKind tk,
                                     std::string msg) {
  Lexer lexer;
  if (peek(psr).kind == TokenKind::END_OF_FILE) {
    ErrorClass::error(current(psr).line, current(psr).column, msg, "",
                      "Parser Error", node.current_file, lexer, psr->tks, true,
                      false, false, false, false, false);
    ErrorClass::printError();
    exit(1);
  }
  if (current(psr).kind != tk) {
    ErrorClass::error(current(psr).line, current(psr).column, msg, "",
                      "Parser Error", node.current_file, lexer, psr->tks, true,
                      false, false, false, false, false);
    return current(psr);
  }

  return advance(psr);
}