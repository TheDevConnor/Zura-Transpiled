#pragma once

#include <string>

class Flags {
public:
  static void runFile(const char *path, std::string outName, bool save, bool debug);
  static char *readFile(const char *path);
  static void updateProgressBar(double progress);
};
