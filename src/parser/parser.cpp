#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "../lexer/lexer.hpp"

#include <string>
#include <vector>

Lexer lexer;

Parser::PStruct *Parser::setupParser(PStruct *psr, Lexer *lex, Lexer::Token tk, std::string current_file) {
  std::unordered_map<std::string, std::string> errors = {};

  while (true) {
    psr->tks.push_back(tk);
    tk = lex->scanToken();
    if (tk.kind == TokenKind::END_OF_FILE) {
      psr->tks.push_back(tk);
      break;
    };
  }

  psr->current_file = current_file;
  node.current_file = current_file;
  return new PStruct{psr->tks, psr->current_file, psr->pos};
}

Node::Stmt *Parser::parse(const char *source, std::string file) {
  PStruct psr;

  // Initialize the lexer and store the tokens
  lexer.initLexer(source, file);
  Parser::PStruct *vect_tk = setupParser(&psr, &lexer, lexer.scanToken(), file);
  ErrorClass::printError();

  createMaps();
  createTypeMaps();
  std::vector<Node::Stmt *> stmts = {};

  while (vect_tk->hadTokens(vect_tk)) {
    stmts.push_back(parseStmt(vect_tk, ""));
    if (vect_tk->hadTokens(vect_tk))
      if (vect_tk->tks.at(vect_tk->pos).kind == TokenKind::END_OF_FILE)
        break;
  }

  // Copy the tokens to the ast node
  node.tks = vect_tk->tks;
  node.current_file = vect_tk->current_file;

  delete vect_tk;
  return new ProgramStmt(stmts, file);
}
