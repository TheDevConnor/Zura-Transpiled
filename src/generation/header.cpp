#include "gen.hpp"
#include <fstream>

void Gen::headerImport(std::ofstream &file) {
  file << "#include <stdio.h>\n";
  file << "#include <stdint.h>\n";
  file << "#include <stdbool.h>\n\n";
}
