#pragma once

#include <iostream>
#include <string>

#ifdef __linux__
#include <sys/stat.h>
#endif

void installer() {
  std::string os_name = "";
#ifdef _WIN32
  os_name = "windows";
#elif __linux__
  os_name = "linux";
#elif __APPLE__
  os_name = "darwin";
#endif

  if (os_name == "windows") {
    std::string url =
        "https://github.com/TheDevConnor/Zura-Transpiled/releases/download/";
    url.append("pre-release/zura.exe");
    std::string command = "curl -LO " + url;
    try {
      system(command.c_str());
    } catch (const std::exception &e) {
      std::cout << "Error running command: " << e.what() << std::endl;
      return;
    }
  } else if (os_name == "linux" || os_name == "darwin") {
    std::string url =
        "https://github.com/TheDevConnor/Zura-Transpiled/releases/download/";
    url.append("pre-release/zura");
    std::string command = "curl -LO " + url;
    try {
      system(command.c_str());
      chmod("zura", 0777);
      command = "sudo mv zura /usr/bin";
      std::cout
          << "Please enter your password to install Zura. For the first time"
          << std::endl;
      system(command.c_str());
    } catch (const std::exception &e) {
      std::cout << "Error running command: " << e.what() << std::endl;
      return;
    }
  } else {
    return;
  }
}
