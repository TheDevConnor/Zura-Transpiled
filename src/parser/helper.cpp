#include "parser.hpp"

bool Parser::PStruct::hadTokens(PStruct *psr) {
  return psr->pos < psr->tks.size();
}

Lexer::Token Parser::PStruct::current(PStruct *psr) {
  return psr->tks[psr->pos];
}

Lexer::Token Parser::PStruct::advance(PStruct *psr) {
  return psr->tks[psr->pos++];
}

Lexer::Token Parser::PStruct::peek(PStruct *psr, int offset) {
  return psr->tks[psr->pos + (size_t)offset];
}

Lexer::Token Parser::PStruct::expect(PStruct *psr, TokenKind tk, std::string msg) {
  if (peek(psr, 0).kind == tk) return advance(psr);
  ErrorClass::error(current(psr).line, current(psr).column, msg,
                    "", "Parser Error", psr->current_file.c_str(),
                    lexer, psr->tks, true, false, false, false, false, false);
  return advance(psr);
}