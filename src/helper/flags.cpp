#include <string>
#include <fstream>
#include <iostream>

// #include "../parser/parser.hpp"
// #include "../type/type.hpp"
// #include "../ast/ast.hpp"
#include "../common.hpp"
#include "../lexer/lexer.hpp"
#include "flags.hpp"

using namespace std;

void Flags::compilerDelete(char **argv) {
  Exit(ExitValue::FLAGS_PRINTED);
}

void Flags::compile(std::string name) {}

void Flags::outputFile(const char *path) {}

char *Flags::readFile(const char *path) {
  ifstream file(path, ios::binary);
  if (!file) {
    cerr << "Error: Could not open file '" << path << "'" << endl;
    Exit(ExitValue::INVALID_FILE);
  }

  file.seekg(0, ios::end);
  size_t size = file.tellg();
  file.seekg(0, ios::beg);

  char *buffer = new char[size + 1];
  file.read(buffer, size);
  file.close();

  buffer[size] = 0;
  return buffer;
}

void Flags::runFile(const char *path, std::string outName, bool save) {
  const char *source = readFile(path);
  
  Lexer lexer;
  lexer.initLexer(source);
  while (lexer.token.kind != TokenKind::END_OF_FILE) {
    lexer.scanToken();
    std::cout << lexer.token.kind << " " << lexer.token.value << std::endl;
  }

  delete[] source;
}
