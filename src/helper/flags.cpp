#include <cstring>
#include <fstream>
#include <iostream>

#include "../ast/ast.hpp"
#include "../common.hpp"
#include "../generation/gen.hpp"
#include "../parser/parser.hpp"
#include "flags.hpp"

using namespace std;

void Flags::compilerDelete(char **argv) {
  cout << "Deleting the executable file" << endl;
  std::string outName = argv[2];
  char rmCommand[256];

#ifdef _WIN32
  strcat(rmCommand, "del ");
  strcat(rmCommand, outName.c_str());
  strcat(rmCommand, ".exe");
  strcat(rmCommand, " out.c");
  system(rmCommand);
#else
  strcat(rmCommand, "rm -rf ");
  strcat(rmCommand, outName.c_str());
  strcat(rmCommand, " out.c");
  system(rmCommand);
#endif
  Exit(ExitValue::FLAGS_PRINTED);
}

void Flags::compileToC(std::string name) {
  std::string compileCommand = "gcc -o " + name + " out.c";
  system(compileCommand.c_str());
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

  Parser parser(source);
  AstNode *expression = parser.parse();
  expression->printAst(expression, 0);

  // TODO: Implement a type checker
  // Type type(expression);

//   Gen gen(expression);

//   compileToC(outName);

//   if (!save) {
// #ifdef _WIN32
//     system("del out.c");
// #else
//     system("rm -rf out.c");
// #endif
//   }

  delete[] source;
  delete expression;
}
