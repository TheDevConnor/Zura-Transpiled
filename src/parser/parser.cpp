#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "parser.hpp"

#include <vector>

Lexer lexer;

using namespace Parser;

std::string Parser::formCurrentLine(const PStruct *psr) {
    std::string currentLine;
      int currentLineNum = psr->tks[psr->pos].line;

      for (size_t i = 0; i <= psr->pos; ++i) {
         if (psr->tks[i].line == currentLineNum) {
               currentLine += psr->tks[i].value + " ";
         }
      }

      return currentLine;
}

Parser::PStruct *Parser::setupParser(PStruct *psr, Lexer *lex,
                                     Lexer::Token tk) {
   while (tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        tk = lex->scanToken();
   }

   std::vector<std::string> errors = {};
   return new Parser::PStruct { errors, psr->tks, 0};
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
      stmts.push_back(parseStmt(vect_tk, ""));

   if (psr.errors.size() > 0) {
      std::cout << "Errors found in the program!" << std::endl;
      for (auto &err : psr.errors) {
         std::cout << err << std::endl;
      }
   }
   
   delete vect_tk;
   return new ProgramStmt(stmts);
}
