#include "../helper/error/error.hpp"
#include "parser.hpp"

bool Parser::PStruct::hadTokens() {
  return pos < tks.size();
}

Lexer::Token Parser::PStruct::current() {
  return tks[pos];
}

Lexer::Token Parser::PStruct::advance() {
  return tks[pos++];
}

Lexer::Token Parser::PStruct::peek(int offset) {
  return tks[pos + (size_t)offset];
}

Lexer::Token Parser::PStruct::expect(TokenKind tk, std::string msg) {
  if (peek(0).kind == tk) return advance();
  ErrorClass::error(current().line, current().column, msg,
                    "", "Parser Error", current_file.c_str(),
                    lexer, tks, true, false, false, false, false, false);
  return advance();
}
