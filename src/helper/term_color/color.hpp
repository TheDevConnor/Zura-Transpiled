#pragma once

#include <string>
#include <unordered_map>

class Color {
public:
  enum C { RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE, BLACK, GRAY };

  std::string color(std::string text, C color, bool isUnderline = false, bool isBold = false) {
    if (!terminal_supports_color())
      return text;
    if (isUnderline)
      return colorCode(color) + "\033[4m" + text + "\033[0m";
    if (isBold)
      return colorCode(color) + "\033[1m" + text + "\033[0m";
    return colorCode(color) + text + "\033[0m";
  }

private:
  std::unordered_map<C, std::string> colorMap = {
      {RED, "\033[31m"},   {GREEN, "\033[32m"},   {BLUE, "\033[34m"},
      {CYAN, "\033[36m"},  {MAGENTA, "\033[35m"}, {YELLOW, "\033[33m"},
      {WHITE, "\033[37m"}, {BLACK, "\033[30m"},  {GRAY, "\033[90m"},
  };

  std::string colorCode(C color) {
    if (colorMap.find(color) != colorMap.end())
      return colorMap[color];
    return "";
  }

  //   8 → basic color support
  //  16 → extended (like bold variants)
  // 256 → full 256-color support
  // 0 or error → no color support (or not a terminal)

  bool terminal_supports_color() {
      const char* term = std::getenv("TERM");
      if (!term || std::string(term) == "dumb")
          return false;

      FILE* pipe = popen("tput colors", "r");
      if (!pipe)
          return false;

      char buffer[128];
      if (fgets(buffer, sizeof(buffer), pipe) == nullptr) {
          pclose(pipe);
          return false;
      }

      pclose(pipe);
      int colors = std::atoi(buffer);
      return colors >= 8;
  }
};