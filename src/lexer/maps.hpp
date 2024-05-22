#include <unordered_map>
#include <functional>
#include <string>

#include "lexer.hpp"

using WhiteSpaceFunction = std::function<void(Lexer&)>;
const std::unordered_map<char, WhiteSpaceFunction> whiteSpaceMap = {
    {' ',  [](Lexer &lexer) { lexer.advance(); }},
    {'\r', [](Lexer &lexer) { lexer.advance(); }},
    {'\t', [](Lexer &lexer) { lexer.advance(); }},
    {'\n', [](Lexer &lexer) {
      lexer.scanner.line++;
      lexer.scanner.column = 0;
      lexer.advance();
    }},
    {'#', [](Lexer &lexer) {
      while (lexer.peek() != '\n' && !lexer.isAtEnd()) lexer.advance();
    }},
};

const std::unordered_map<std::string, TokenKind> keywords = {
      {"and", TokenKind::AND},
      {"class", TokenKind::CLASS},
      {"else", TokenKind::ELSE},
      {"false", TokenKind::FAL},
      {"fn", TokenKind::FUN},
      {"loop", TokenKind::LOOP},
      {"if", TokenKind::IF},
      {"nil", TokenKind::NIL},
      {"or", TokenKind::OR},
      {"dis", TokenKind::PRINT},
      {"return", TokenKind::RETURN},
      {"exit", TokenKind::EXIT},
      {"super", TokenKind::SUPER},
      {"this", TokenKind::THS},
      {"true", TokenKind::TR},
      {"have", TokenKind::VAR},
      {"pkg", TokenKind::PKG},
      {"type", TokenKind::TYPE},
      {"struct", TokenKind::STRUCT},

      // Types.
      {"i8", TokenKind::I8},
      {"i16", TokenKind::I16},
      {"i32", TokenKind::I32},
      {"i64", TokenKind::I64},
      {"i128", TokenKind::I128},
      {"u8", TokenKind::U8},
      {"u16", TokenKind::U16},
      {"u32", TokenKind::U32},
      {"u64", TokenKind::U64},
      {"u128", TokenKind::U128},
      {"f32", TokenKind::F32},
      {"f64", TokenKind::F64},
      {"f128", TokenKind::F128},
      {"bool", TokenKind::Bool},
      {"char", TokenKind::Char},
      {"str", TokenKind::STRING},
};

const std::unordered_map<char, TokenKind> scMap = {
    {'(', TokenKind::LEFT_PAREN},
    {')', TokenKind::RIGHT_PAREN},
    {'{', TokenKind::LEFT_BRACE},
    {'}', TokenKind::RIGHT_BRACE},
    {';', TokenKind::SEMICOLON},
    {',', TokenKind::COMMA},
    {'.', TokenKind::DOT},
    {'-', TokenKind::MINUS},
    {'+', TokenKind::PLUS},
    {'/', TokenKind::SLASH},
    {'*', TokenKind::STAR},
    {'%', TokenKind::MODULO},
    {'^', TokenKind::CARET},
    {'[', TokenKind::LEFT_BRACKET},
    {']', TokenKind::RIGHT_BRACKET},
    {':', TokenKind::COLON},
    {'=', TokenKind::EQUAL},
    {'!', TokenKind::BANG},
    {'<', TokenKind::LESS},
    {'>', TokenKind::GREATER},
};

const std::unordered_map<std::string, TokenKind> dcMap = {
      {"!=", TokenKind::BANG_EQUAL},
      {"==", TokenKind::EQUAL_EQUAL},
      {">=", TokenKind::GREATER_EQUAL},
      {"<=", TokenKind::LESS_EQUAL},
      {":=", TokenKind::WALRUS},
};