#include <iostream>
#include <string>

#include "../../../inc/colorize.hpp"
#include "../../lexer/lexer.hpp"
#include "../../common.hpp"
#include "error.hpp"

void ErrorClass::printIgnoreSpaces(int line) {
    if (line < 10) std::cout << " ";
    if (line >= 10 && line < 100) std::cout << "  ";
    if (line >= 100) std::cout << "   ";
    if (line >= 1000) std::cout << "    ";
}

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

void ErrorClass::beforeLine(int line, Lexer &lexer) {
    if (line < 1) return;
    const char *start = lexer.lineStart(line);
    printLine(line, start);
}

void ErrorClass::currentLine(int line, int pos, Lexer &lexer) {
    const char *start = lexer.lineStart(line);
    printLine(line, start);
}

void ErrorClass::afterLine(int line, Lexer &lexer) {
    const char *start = lexer.lineStart(line);
    printLine(line, start);
}

void ErrorClass::error(int line, int pos, std::string msg, std::string errorType, 
                       std::string filename, Lexer &lexer) {
    int cPos = pos;
    if (line == 1) cPos += 1;

    std::cout << "[" << line << "::" << cPos << "] ("
              << filename << ") \n ↳ " << termcolor::red 
              << errorType << termcolor::reset << " " << msg << std::endl;

    beforeLine(line - 1, lexer);
    currentLine(line, cPos, lexer);
    afterLine(line + 1, lexer);

    std::cout << std::endl;
}