#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>
#include <sys/stat.h>

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
      "version"
      "\n Compiler Flags:\n  build [file]  Build a Zura file"
      "\n  -name [name]  Set the name of the output file"
      "\n  -save [path]  Save the output file to a specific path"
      "\n  -clean        Clean the build files [*.asm, *.o]",
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
  using conditions = bool (*)(const char *);
  conditions buildConditions[] = {
      [](const char *arg) { return strcmp(arg, "build") == 0; },
      [](const char *arg) { return strcmp(arg, "-name") == 0; },
      [](const char *arg) { return strcmp(arg, "-save") == 0; },
      [](const char *arg) { return strcmp(arg, "-clean") == 0; },
  };

  if (argc < 2) {
    std::cout << "No arguments provided" << std::endl;
    Exit(ExitValue::INVALID_FILE);
  }

  for (int i = 0; i < 4; i++) {
    if (buildConditions[i](argv[1])) {
      if (i == 3) { // clean
        std::string remove = "rm *.asm *.o";
        system(remove.c_str());
        return;
      }
      if (i == 0) { // build
        if (argc < 3) {
          std::cout << "No file specified" << std::endl;
          Exit(ExitValue::BUILD_ERROR);
        }

        const char *fileName = argv[2];
        const char *outputName = "a.out";
        bool saveFlag = false;

        // Check for additional flags after 'build'
        for (int j = 3; j < argc; ++j) {
          if (strcmp(argv[j], "-name") == 0) {
            if (j + 1 < argc) {
              outputName = argv[j + 1];
              ++j; // Skip the output name argument
            } else {
              std::cout << "No output name specified" << std::endl;
              Exit(ExitValue::INVALID_FILE);
            }
          } else if (strcmp(argv[j], "-save") == 0) {
            saveFlag = true;
          }
        }

        Flags::runFile(fileName, outputName, saveFlag);
        return; // Exit after handling the 'build' command
      }
    }
  }
}

void updateProgressBar(double progress) {
    const int barWidth = 50;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

int main(int argc, char **argv) {
    auto startTime = std::chrono::high_resolution_clock::now();

    FlagConfig::print(argc, argv);
    auto midTime = std::chrono::high_resolution_clock::now();

    updateProgressBar(0.5);

    FlagConfig::runBuild(argc, argv);
    auto endTime = std::chrono::high_resolution_clock::now();

    updateProgressBar(1.0);
    std::cout << std::endl;

    auto totalDurationMS = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    auto totalDurationUS = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    std::cout << "Total time: " << totalDurationMS << "ms (" << totalDurationUS << "us)" << std::endl; 

    return ExitValue::BUILT;
} 