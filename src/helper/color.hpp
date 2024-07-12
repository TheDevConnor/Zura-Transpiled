#pragma once

#include <string>

class Color {
public:
    enum C {
        RED,
        GREEN,
        BLUE,
        CYAN,
        MAGENTA,
        YELLOW,
        WHITE,
        BLACK
    };

    std::string color(std::string text, C color) {
        if (terminal_supports_color()) return text;
        return colorCode(color) + text + "\033[0m";
    } 
private:
    std::string colorCode(C color) {
        switch (color) {
        case RED:
            return "\033[31m";
        case GREEN:
            return "\033[32m";
        case BLUE:
            return "\033[34m";
        case CYAN:
            return "\033[36m";
        case MAGENTA:
            return "\033[35m";
        case YELLOW:
            return "\033[33m";
        case WHITE:
            return "\033[37m";
        case BLACK:
            return "\033[30m";
        default:
            return "";
        }
    }

    bool terminal_supports_color() {
        FILE* pipe = popen("/bin/sh -c 'tput colors'", "r");
        if (!pipe) return false;

        int result = 0;
        if (pclose(pipe) == -1) return false;

        return result > 0;
    }
};