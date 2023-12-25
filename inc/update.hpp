
#pragma once

#include <fstream>
#include <iostream>
#include <string>

#ifdef __linux__
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
const std::string os_name = "windows";
#elif __linux__
const std::string os_name = "linux";
#elif __APPLE__
const std::string os_name = "darwin";
#else
const std::string os_name = "";
#endif

const std::string urlBase =
    "https://github.com/TheDevConnor/Zura-Transpiled/releases/download/";

void promptUpdate() {
  std::cout << "Zura is already installed on your system. Do you want to "
               "update it? (y/n): Default is y";
  std::string input;
  std::getline(std::cin, input);
  if (input == "n") {
    exit(0);
  }
}

void runSystemCommand(const std::string &command) {
  try {
    system(command.c_str());
  } catch (const std::exception &e) {
    std::cout << "Error running command: " << e.what() << std::endl;
    exit(1);
  }
}

void installer() {
  std::string url = urlBase;

  if (os_name == "windows") {
    url.append("pre-release/zura.exe");

    std::ifstream f("C:\\Windows\\System32\\zura.exe");
    if (f.good()) {
      promptUpdate();
      runSystemCommand("del C:\\Windows\\System32\\zura.exe");
    }

    std::string command = "curl -LO " + url;
    runSystemCommand(command);

    // Prompt the user for admin privileges and move the file to System32
    command = "powershell -Command \"Start-Process cmd -Verb RunAs "
              "-ArgumentList '/c move zura.exe C:\\Windows\\System32\\'\"";
    std::cout
        << "Please enter your password to install Zura. For the first time"
        << std::endl;
    runSystemCommand(command);

    std::cout << "Zura has been installed successfully!" << std::endl;
  } else if (os_name == "linux" || os_name == "darwin") {
    url.append("pre-release/zura");
    std::string command = "curl -LO " + url;

    std::ifstream f("/usr/bin/zura");
    if (f.good()) {
      promptUpdate();
      runSystemCommand("sudo rm /usr/bin/zura");
    }

    runSystemCommand(command);

    chmod("zura", 0777);

    command = "sudo mv zura /usr/bin";
    std::cout
        << "Please enter your password to install Zura. For the first time"
        << std::endl;
    runSystemCommand(command);

    std::cout << "Zura has been installed successfully!" << std::endl;
  } else {
    return;
  }
}
