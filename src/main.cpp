#include <sys/stat.h>

#include <chrono>
#include <cstdlib>  // Include this for std::system
#include <cstring>
#include <iostream>
#include <string>

#include "common.hpp"
#include "helper/flags.hpp"
#include "server/lsp.hpp"

void FlagConfig::print(int argc, char **argv) {
  (void)argc;
  using condition = bool (*)(const char *);
  condition conditions[] = {
      [](const char *arg) { return strcmp(arg, "--version") == 0; },
      [](const char *arg) { return strcmp(arg, "--license") == 0; },
      [](const char *arg) { return strcmp(arg, "--help") == 0; },
      [](const char *arg) { return strcmp(arg, "--update") == 0; },
  };

  // Messages array will be built at runtime
  std::string version_message = "Zura Lang " + ZuraVersion;
  // TODO: make this colorful and nice to look at
  std::string messages[] = {
      version_message.c_str(),
      "Zura uses a license under GPL-3.0\nYou can find the license "
      "here:\nhttps://www.gnu.org/licenses/gpl-3.0.en.html",
      version_message +
          "\nUsage: zura [options] [file]\nOptions:\n  --version    Print the "
          "version of Zura\n  --help       Print this help message\n  --license    "
          "Print the license of Zura\n  --update     Update Zura to the latest "
          "version\n Compiler Flags:\n  build [file]  Build a Zura file"
          "\n  -name [name]  Set the name of the output file"
          "\n  -save [path]  Save the output file to a specific path"
          "\n  -clean        Clean the build files [*.asm, *.o]"
          "\n Zura Lsp Flags:"
          "\n  -lsp          Create an LSP connection via stdio."};

  if (argc <= 1) {
    std::cout << messages[2].c_str() << std::endl;
    return;
  }

  for (int i = 0; i < 4; i++) {
    if (conditions[i](argv[1])) {
      if (i == 3) {
        std::cout << "This feature is not yet implemented" << std::endl;
        Exit(ExitValue::UPDATED);
      }
      std::cout << messages[i] << std::endl;
      Exit(ExitValue::SUCCESS);  // Why would you error when you printed successfully?
    }
  }
}

void FlagConfig::runBuild(int argc, char **argv) {
  using conditions = bool (*)(const char *);
  conditions buildConditions[] = {
      [](const char *arg) { return strcmp(arg, "build") == 0; },
      [](const char *arg) { return strcmp(arg, "-name") == 0; },
      [](const char *arg) { return strcmp(arg, "-save") == 0; },
      [](const char *arg) { return strcmp(arg, "-clean") == 0; },
      [](const char *arg) { return strcmp(arg, "-debug") == 0; },
      [](const char *arg) { return strcmp(arg, "-quiet") == 0; },
      [](const char *arg) { return strcmp(arg, "-lsp") == 0; },
  };

  if (argc == 1) {
    std::cout << "No arguments provided" << std::endl;
    Exit(ExitValue::INVALID_FILE);
  }

  // check if they are running the language server protocol
  if (buildConditions[6](argv[1])) {
    // The LSP command option will bascially take over control of the whole process.
    // We no longer care about any other options.

    // Colored text
    // This was causing bugs because of the unexpected output for the Client. It would have been cool to see though!
    // Color col;
    // std::cout << col.color("Zura " + ZuraVersion + " LSP", Color::C::CYAN, false, true) << "\n";
    // std::cout << col.color("The builtin LSP is made solely for the computers. It is not meant for humans. You have been warned!", Color::C::RED, false, true) << "\n";
    lsp::main();
    return;
  }

  for (int i = 0; i < 4; i++) {
    if (buildConditions[i](argv[1])) {
      if (i == 3) {  // clean
        std::string remove = "rm *.s *.o";
        int exitCode = std::system(remove.c_str());
        if (exitCode && !Flags::quiet) {
          // If non-zero (error)
          std::cout << "No files to clean." << std::endl;
        }
        return;
      }
      if (i == 0) {  // build
        if (argc < 3) {
          std::cerr << "No file specified" << std::endl;
          Exit(ExitValue::BUILD_ERROR);
        }

        const char *fileName = argv[2];  // ! important for linker dir later
        const char *outputName = "out";
        bool saveFlag = false;
        bool isDebug = false;
        bool isQuiet = false;

        // Check for additional flags after 'build'
        // TODO: Remove nested loop ..?!?!?!?
        for (int j = 3; j < argc; ++j) {
          if (strcmp(argv[j], "-name") == 0) {
            if (j + 1 < argc) {
              outputName = argv[j + 1];
              ++j;  // Skip the output name argument
            } else {
              std::cout << "No output name specified" << std::endl;
              Exit(ExitValue::INVALID_FILE);
            }
          } else if (strcmp(argv[j], "-save") == 0) {
            saveFlag = true;
          } else if (strcmp(argv[j], "-debug") == 0) {
            isDebug = true;
          } else if (strcmp(argv[j], "-quiet") == 0) {
            Flags::quiet = isQuiet = true;
          }
        }

        Flags::runFile(fileName, outputName, saveFlag, isDebug, !isQuiet);
        return;  // Exit after handling the 'build' command
      }
    }
  }
}

int main(int argc, char **argv) {
  // TODO: Ensure this file can be stored somewhere actually secure, like system files or in a .zurarc file

  std::chrono::time_point startTime = std::chrono::high_resolution_clock::now();

  if (!Flags::quiet) FlagConfig::print(argc, argv);
  // std::chrono::time_point midTime = std::chrono::high_resolution_clock::now();

  FlagConfig::runBuild(argc, argv);
  std::chrono::time_point endTime = std::chrono::high_resolution_clock::now();

  if (!Flags::quiet) {
    Flags::updateProgressBar(1.0);
    std::cout << std::endl;

    int64_t totalDurationMS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    int64_t totalDurationUS = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "Total time: " << totalDurationMS << "ms (" << totalDurationUS << "µs)" << std::endl;
  }

  return ExitValue::BUILT;
}
