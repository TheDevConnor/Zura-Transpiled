#pragma once
#ifdef csk

#include <string>

class Flags {
public:
  static void runFile(const char *path, std::string outName, bool save);
  static void outputFile(const char *path);
  static void compile(std::string name);
  static void compilerDelete(char **argv);
  static char *readFile(const char *path);
};
#endif