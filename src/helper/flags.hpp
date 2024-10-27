#pragma once

#include <string>

class Flags {
public:
  static void runFile(const char *path, std::string outName, bool save, bool debug, bool echoOn);
  static char *readFile(const char *path);
  static void updateProgressBar(double progress);

  static inline bool quiet = false;
};
