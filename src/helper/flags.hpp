#pragma once

#include <string>

class Flags {
public:
  static void runFile(const char *path, std::string outName, bool save);
  static void compileToAsm(std::string name);
  static void compilerDelete(char **argv);
  static char *readFile(const char *path);
};
