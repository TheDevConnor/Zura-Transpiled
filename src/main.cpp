#include <cstring>
#include <iostream>

#include "../inc/colorize.hpp"
#include "../inc/update.hpp"
#include "common.hpp"
#include "helper/flags.hpp"

using namespace std;

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
      cout << messages[i] << endl;
      Exit(ExitValue::FLAGS_PRINTED);
    }
  }
}

void FlagConfig::runBuild(int argc, char **argv) {
  if (argc == 2) {
    cout << "No file specified" << endl;
    Exit(ExitValue::INVALID_FILE);
  }
  if (argc == 3 && strcmp(argv[1], "build") == 0) {
    Flags::runFile(argv[2], "a.out", false);
  }
}

void FlagConfig::createFlagsMap(int argc, char **argv) {
  FlagsMap = {
      {"--version", print}, {"--help", print},   {"--license", print},
      {"--update", print},  {"build", runBuild},
  };
}

int main(int argc, char **argv) {
  flagConfig.createFlagsMap(argc, argv);

  for (auto &flag : flagConfig.FlagsMap) {
    if (strcmp(argv[1], flag.first.c_str()) == 0) {
      flag.second(argc, argv);
    } 
  }

  return 0;
}