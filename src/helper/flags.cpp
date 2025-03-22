#include "flags.hpp"
#include "../common.hpp"
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include "../typeChecker/type.hpp"
#include "../codegen/gen.hpp"
#include "../codegen/optimizer/compiler.hpp"
#include "error/error.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

void Flags::updateProgressBar(double progress) {
    const int barWidth = 50;
    std::cout << "\033[2K\r[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
}

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

void Flags::runFile(const char *path, std::string outName, bool save, bool debug, bool echoOn) {
  const char *source = readFile(path);

  if (echoOn) Flags::updateProgressBar(0.0);
  Node::Stmt *result = Parser::parse(source, path);
  bool parserError = ErrorClass::printError();
  if (parserError) Exit(ExitValue::PARSER_ERROR); 
  if (echoOn) Flags::updateProgressBar(0.25);

  TypeChecker::performCheck(result);
  bool tcError = ErrorClass::printError();
  if (tcError) Exit(ExitValue::TYPE_ERROR);
  if (echoOn) Flags::updateProgressBar(0.5);
  
  // Compiler optimize the AST!
  result = CompileOptimizer::optimizeStmt(result);
  if (echoOn) Flags::updateProgressBar(0.75);

  codegen::gen(result, save, outName, path, debug);
  bool codegenError = ErrorClass::printError();
  if (codegenError) Exit(ExitValue::GENERATOR_ERROR);
  if (echoOn) Flags::updateProgressBar(1.0); // We're done!

  delete[] source;
  delete result;
}