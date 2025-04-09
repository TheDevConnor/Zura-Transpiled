#pragma once
#include <fstream>

namespace logging {
  inline std::ofstream file;
  inline void init() {
    if (!file.is_open())
      file.open("lsp.log");
  }

  inline void log(std::string message) {
    file << message << std::flush;
  }

  inline void log(char message) {
    file << message << std::flush;
  }

  inline void close() {
    if (file.is_open())
      file.close();
  }
}