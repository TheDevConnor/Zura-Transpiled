#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "parser.hpp"

#include <vector>

Lexer lexer;

Parser::PStruct *Parser::setupParser(PStruct *psr, Lexer *lex,
                                     Lexer::Token tk) {
   while (tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        tk = lex->scanToken();
   }
   
   return new Parser::PStruct { psr->tks, 0 };
}

Node::Stmt *Parser::parse(const char *source) {
   PStruct psr;

   // Initialize the lexer and store the tokens
   lexer.initLexer(source);
   auto vect_tk = setupParser(&psr, &lexer, lexer.scanToken());

   createMaps();
   createTypeMaps();
   auto stmts = std::vector<Node::Stmt *>();

   while (vect_tk->hadTokens(vect_tk)) 
      stmts.push_back(parseStmt(vect_tk));
   
   delete vect_tk;
   return new ProgramStmt(stmts);
}
