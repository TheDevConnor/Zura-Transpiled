#pragma once
#include <stdlib.h>

#include <string>

inline std::string ZuraVersion = "v0.1.42";
inline bool shouldPrintErrors = true;

class FlagConfig {
 public:
  static void print(int argc, char **argv);
  static void runBuild(int argc, char **argv);
};

enum ExitValue {
  BUILT = 0,
  SUCCESS = 0,
  INVALID_FILE_EXTENSION = 1,
  INVALID_FILE = 2,
  LEXER_ERROR = 3,
  PARSER_ERROR = 4,
  _ERROR = 5,
  GENERATOR_ERROR = 6,
  UPDATED = 7,
  INVALID_TYPE = 8,
  TYPE_ERROR = 9,
  BUILD_ERROR = 10,
  FLAGS_PRINTED = 11,
};

inline void Exit(ExitValue exitValue) { exit(ExitValue(exitValue)); }
inline FlagConfig flagConfig;
