#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"
#include "parser.hpp"

#include <vector>

Lexer lexer;

std::vector<Lexer::Token> ParserNamespace::storeToken(Parser *psr, Lexer *lex, 
                                                      Lexer::Token tk) {
   while (tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        tk = lex->scanToken();
   } 
   return psr->tks;
}

ParserNamespace::Parser *ParserNamespace::createParser(std::vector<Lexer::Token> tks) {
   createTokenLookup();
   return new ParserNamespace::Parser { tks, 0 }; 
}

Node::Stmt *ParserNamespace::parse(const char *source) {
   Parser psr;

   // Initialize the lexer and store the tokens
   lexer.initLexer(source);
   auto vect_tk = storeToken(&psr, &lexer, lexer.scanToken());

   std::cout << "Tokens: \n";
   for (auto &tk : vect_tk) {
       std::cout << tk.value << std::endl;
   }

   // Create the parser
   auto parser = createParser(vect_tk);
   std::cout << "Parser created\n";

   auto stmts = std::vector<Node::Stmt *>();
   std::cout << "Parsing statements\n";

   while (parser->hasTokens(&psr)) {
      std::cout << "Parsing statement (Inside Loop)\n";
      auto stmt = parseStmt(&psr);
      stmts.push_back(stmt); 
   }
   return new ProgramStmt(stmts);
}
