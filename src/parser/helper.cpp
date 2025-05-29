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
  // C++ runtime errors :)
  if (!hadTokens()) {
    std::string message = "Expected token of type '" + std::string(Lexer::tokenToStringMap.at(tk)) + "', but instead found the end of the file.";
    Error::handle_error("Parser", current_file, message, tks,
                        tks.back().line, tks.back().column);
    return tks.back();
  }
  if (peek(0).kind == tk) return advance();
  std::string errorMsg = msg + " Found: " + peek(0).value;
  Error::handle_error("Parser", current_file, errorMsg, tks, current().line, current().column);
  return current();
}
