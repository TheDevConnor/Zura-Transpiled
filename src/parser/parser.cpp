#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"
#include <cstdlib>

Lexer lexer;

void ParserClass::storeToken(Parser *psr, Lexer *lex, Lexer::Token tk) {
   while (tk.kind != TokenKind::END_OF_FILE) {
        psr->tks.push_back(tk);
        tk = lex->scanToken();
   } 
}

Node::Stmt ParserClass::parse(const char *source) {
   Parser psr;

   // Initialize the lexer and store the tokens
   lexer.initLexer(source);
   storeToken(&psr, &lexer, lexer.scanToken());

    while (psr.pos < psr.tks.size()) {
        std::cout << "Token: " << psr.current(&psr).value << std::endl;
        psr.advance(&psr);
    }

//    auto expr = ParserClass::parseExpr(&psr, BindingPower::defaultValue);

   while (psr.pos < psr.tks.size()) {
      std::cout << "Token: " << psr.tks[psr.pos].value << std::endl;
      psr.pos++;
   }
   
   return Node::Stmt();
}
