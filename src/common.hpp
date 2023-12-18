#pragma once
#include <stdlib.h>
#define ZuraVersion "v0.0.1"

enum ExitValue {
  FLAGS_PRINTED = 0,
  INVALID_FILE_EXTENSION = 1,
  INVALID_FILE = 2,
  LEXER_ERROR = 3,
  PARSER_ERROR = 4,
  GENERATOR_ERROR = 5,
  UPDATED = 6,
};

void installer();
inline void Exit(ExitValue exitValue) { exit(ExitValue(exitValue)); }
