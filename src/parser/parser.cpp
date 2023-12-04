#include "../helper/error/parserError.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "parser.hpp"

Parser::Parser(Lexer::Token token, bool error) : token(token) {}
Parser::~Parser() {}