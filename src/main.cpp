#include <cstring>
#include <iostream>

#include "../inc/update.hpp"
#include "common.hpp"
#include "helper/flags.hpp"

void FlagConfig::print(int argc, char **argv) {
  using condition = bool (*)(const char *);
  condition conditions[] = {
      [](const char *arg) { return strcmp(arg, "--version") == 0; },
      [](const char *arg) { return strcmp(arg, "--license") == 0; },
      [](const char *arg) { return strcmp(arg, "--help") == 0; },
      [](const char *arg) { return strcmp(arg, "--update") == 0; },
  };
  const char *messages[] = {
      "Zura Lang " ZuraVersion,
      "Zura uses a license under GPL-3.0\nYou can find the license "
      "here:\nhttps://www.gnu.org/licenses/gpl-3.0.en.html",
      "Zura Lang " ZuraVersion
      "\nUsage: zura [options] [file]\nOptions:\n  --version    Print the "
      "version of Zura\n  --help       Print this help message\n  --license    "
      "Print the license of Zura\n  --update     Update Zura to the latest "
      "version" "\n Compiler Flags:\n  build [file]  Build a Zura file",
  };

  for (int i = 0; i < 4; i++) {
    if (conditions[i](argv[1])) {
      if (i == 3) {
        promptUpdate();
        Exit(ExitValue::UPDATED);
      }
      std::cout << messages[i] << std::endl;
      Exit(ExitValue::FLAGS_PRINTED);
    }
  }
}

void FlagConfig::runBuild(int argc, char **argv) {
  if (argc == 2) {
    std::cout << "No file specified" << std::endl;
    Exit(ExitValue::INVALID_FILE);
  }
  if (argc == 3 && strcmp(argv[1], "build") == 0) {
    Flags::runFile(argv[2], "a.out", false);
  }
}

int main(int argc, char **argv) {
  FlagConfig::print(argc, argv);
  FlagConfig::runBuild(argc, argv);
  return 0;
}