#include <iostream>
#include <vector>
#include <string>

#include "../../parser/parser.hpp"
#include "../../lexer/lexer.hpp"
#include "../color.hpp"
#include "../../common.hpp"
#include "error.hpp"

Color col;

std::string ErrorClass::lineNumber(int line) {
    return (line < 10) ? "0" : "";
}

std::string ErrorClass::printLine(int line, const char *start) {
    const char *end = start;
    while (*end != '\n' && *end != '\0') end++;
    return lineNumber(line) + std::to_string(line) + " | " + std::string(start, end - start) + "\n";
}

// TODO: Think about how to refactor this function so that we can have the pointer on a new line in the correct position
std::string ErrorClass::currentLine(int line, int pos, Lexer &lexer, bool isParser, std::vector<Lexer::Token> tokens) {
    if (isParser) {
        std::string currentLine = lineNumber(line) + std::to_string(line) + " | ";
        for (const auto& tk : tokens) {
            if (tk.line == line) {
                if (tk.column == pos) currentLine += col.color("_", Color::RED, true, true);
                currentLine += tk.value + " ";
            }
        }
        return currentLine;
    }
    const char *start = lexer.lineStart(line);
    return printLine(line, start);
}

std::string ErrorClass::error(int line, int pos, std::string msg, std::string note, std::string errorType, 
                       std::string filename, Lexer &lexer, std::vector<Lexer::Token> tokens,
                       bool isParser, bool isWarning, bool isFatal, bool isMain) {
    std::string line_error = "[" + std::to_string(line) + "::" + std::to_string(pos) + "] (";
    line_error += (isWarning) ? col.color("Warning", Color::YELLOW, false, true) : 
                    (isFatal) ? col.color("Fatal", Color::RED, false, true) : 
                    col.color("Error", Color::RED);
    line_error += ") (" + filename + ")\n ↳ " + errorType + " " + msg + "\n"; 

    if (isMain) {
        line_error += "\t" + col.color("Note", Color::CYAN, false, true) + ": ";
        line_error += note + "\n";
        if (errors.find(line) == errors.end()) errors[line] = line_error;
        return line_error;
    }

    if (note != "") line_error += " ↳ " + note + "\n";

    line_error += currentLine(line, pos, lexer, isParser, tokens);
    line_error += "\n";

    if (errors.find(line) == errors.end()) errors[line] = line_error;
    return line_error;
}

void ErrorClass::printError() {
    if (errors.size() > 0) {
      std::cout << "Total number of Errors: " << col.color(std::to_string(errors.size()), Color::RED, true, false) << std::endl;
      for (const auto& [line, errMsg] : errors) {
         std::cout << errMsg << std::endl;
      }
      Exit(ExitValue::_ERROR);
   }
}