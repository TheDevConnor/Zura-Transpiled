#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "parser.hpp"

#include <vector>

Lexer lexer;

void ParserNamespace::storeToken(Parser *psr, Lexer *lex, Lexer::Token tk) {
   while (tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        tk = lex->scanToken();
   } 
}

Node::Stmt *ParserNamespace::parse(const char *source) {
   Parser psr;

   // Initialize the lexer and store the tokens
   lexer.initLexer(source);
   storeToken(&psr, &lexer, lexer.scanToken());

   auto stmts = std::vector<Node::Stmt *>{ stmtHandler(&psr) };
   
   return new ProgramStmt({ stmts });
}
