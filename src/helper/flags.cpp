
#include "flags.hpp"
#include "../common.hpp"
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include "../typeChecker/type.hpp"
#include "../codegen/gen.hpp"
#include "error/error.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

void Flags::updateProgressBar(double progress) {
    const int barWidth = 50;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
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

void Flags::runFile(const char *path, std::string outName, bool save) {
  const char *source = readFile(path);
  Flags::updateProgressBar(0.25);
  auto result = Parser::parse(source, path);
  ErrorClass::printError();
  // std::cout << "Passed Parsing" << std::endl;

  Flags::updateProgressBar(0.5);
  TypeChecker::performCheck(result);
  ErrorClass::printError();
  // std::cout << "Passed Type Checking" << std::endl;
  Flags::updateProgressBar(0.75);
  codegen::gen(result, save, outName);
  ErrorClass::printError();
  // std::cout << "Passed Code Generation" << std::endl;
  delete[] source;
  delete result;
}
