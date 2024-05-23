#include <iostream>
#include <string>

#include "../../../inc/colorize.hpp"
#include "../../lexer/lexer.hpp"
#include "../../common.hpp"
#include "error.hpp"

/*
    TODO: Need to fix this. 
    [3::29] (main.zu)
    ↳ Lexer Error Unexpected character.
    [1]    10243 segmentation fault (core dumped)  ./build/zura test.zu -o main
    We are segfaulting when trying to get the value from the start 
    value in currentLine, beforeLine, and afterLine.
*/

Lexer lexer;

const char *ErrorClass::lineNumber(int line) {
    if (line < 10) return "0";
    return "";
}

void ErrorClass::printLine(int line, const char *start) {
    const char *end = start;
    while (*end != '\n' && *end != '\0') end++;

    std::cout << lineNumber(line) << line << " | " << std::string(start, end - start) 
              << std::endl;
}

void ErrorClass::beforeLine(int line) {
    if (line < 1) return;
    const char *start = lexer.lineStart(line);
    printLine(line, start);
}

void ErrorClass::currentLine(int line, int pos) {
    const char *start = lexer.lineStart(line);
    printLine(line, start);
}

void ErrorClass::afterLine(int line) {
    const char *start = lexer.lineStart(line);
    printLine(line, start);
}

void ErrorClass::error(int line, int pos, std::string msg, std::string errorType, std::string filename) {
    int cPos = pos;
    if (line == 1) cPos += 1;

    std::cout << "[" << line << "::" << cPos << "] ("
              << filename << ") \n ↳ " << termcolor::red 
              << errorType << termcolor::reset << " " << msg << std::endl;

    // beforeLine(line - 1);
    currentLine(line, cPos);
    // afterLine(line + 1);
}