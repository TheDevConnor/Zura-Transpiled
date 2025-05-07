#include "parser.hpp"

#include <string>
#include <vector>

#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "../helper/error/error.hpp"
#include "../lexer/lexer.hpp"

Node::Stmt *Parser::parse(const char *source, std::string file) {
  lexer.initLexer(source, file);
  std::vector<Lexer::Token> tks;
  while (true) {
    Lexer::Token tk = lexer.scanToken();
    if (tk.kind == TokenKind::END_OF_FILE) break;
    tks.push_back(tk);
  }

  PStruct psr = PStruct{tks, file, 0};
  node.current_file = file;
  if (Error::report_error()) return nullptr; // Error handling
  if (tks.empty()) {
    Error::handle_error("Parser", file, "No tokens found!", tks, 0, 0);
    return nullptr;
  }

  createMaps();
  createTypeMaps();

  std::vector<Node::Stmt *> stmts = {};
  while (psr.hadTokens()) stmts.push_back(parseStmt(&psr, ""));

  // Copy the tokens to the ast node
  node.tks = psr.tks;
  node.current_file = psr.current_file;

  return new ProgramStmt(stmts, file);
}
