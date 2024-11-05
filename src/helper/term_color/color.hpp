#pragma once

#include <string>
#include <unordered_map>

class Color {
public:
  enum C { RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE, BLACK };

  std::string color(std::string text, C color, bool isUnderline = false,
                    bool isBold = false) {
    if (terminal_supports_color())
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
      {WHITE, "\033[37m"}, {BLACK, "\033[30m"},
  };

  std::string colorCode(C color) {
    if (colorMap.find(color) != colorMap.end())
      return colorMap[color];
    return "";
  }

  // Check to see if the terminal supports color
  bool terminal_supports_color() {
    FILE *pipe = popen("/bin/sh -c 'tput colors'", "r");
    if (!pipe)
      return false;

    int result = 0;
    if (pclose(pipe) == -1)
      return false;

    return result > 0;
  }
};