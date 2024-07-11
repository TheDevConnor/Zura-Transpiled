#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "parser.hpp"

#include <vector>

Lexer lexer;

using namespace Parser;

std::string Parser::formCurrentLine(Parser::PStruct* psr) {
    int line = psr->tks[psr->pos].line;
    std::string currentLine;
    for (const auto& tk : psr->tks) {
        if (tk.line == line) {
            currentLine += tk.value + " ";
        }
    }
    return currentLine;
}

void printErrors(const Parser::PStruct* psr) {
   if (psr->errors.size() > 0) {
      std::cout << "Total number of Errors: " << psr->errors.size() << std::endl;
      for (const auto& [line, errMsg] : psr->errors) {
         std::cout << errMsg << std::endl;
      }
      exit(15);
   }
}

Parser::PStruct *Parser::setupParser(PStruct *psr, Lexer *lex,
                                     Lexer::Token tk) {
   while (tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        tk = lex->scanToken();
   }

   std::unordered_map<std::string, std::string> errors = {};
   return new Parser::PStruct {psr->tks, psr->pos};
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

   printErrors(vect_tk); 

   delete vect_tk;
   return new ProgramStmt(stmts);
}
