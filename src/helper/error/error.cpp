#include <iostream>
#include <string>
#include <vector>

#include "../../ast/ast.hpp"
#include "../../common.hpp"
#include "../../lexer/lexer.hpp"
#include "../../parser/parser.hpp"
#include "../term_color/color.hpp"
#include "error.hpp"

Color col;

std::string ErrorClass::lineNumber(int line) { return (line < 10) ? "0" : ""; }

std::string ErrorClass::printLine(int line, const char *start) {
  const char *end = start;
  while (*end != '\n' && *end != '\0') {
    end++;
  }
  return lineNumber(line) + std::to_string(line) + " | " +
         std::string(start, end - start) + "\n";
}

std::string
ErrorClass::formatLineWithTokens(int line, int pos,
                                 const std::vector<Lexer::Token> &tokens,
                                 bool highlightPos = false) {
  std::string formattedLine = lineNumber(line) + std::to_string(line) + " | ";
  for (const Lexer::Token &tk : tokens) {
    if (tk.line == line) {
      if (highlightPos && tk.column == pos) {
        formattedLine += col.color("_", Color::RED, true, true);
      }
      formattedLine += tk.value + " ";
    }
  }
  formattedLine += "\n";
  return formattedLine;
}

std::string ErrorClass::currentLine(int line, int pos, Lexer &lexer,
                                    bool isParser, bool isTypeError,
                                    const std::vector<Lexer::Token> &tokens) {
  if (isParser || isTypeError) {
    return formatLineWithTokens(line, pos, tokens, isParser);
  }

  const char *start = lexer.lineStart(line);
  return printLine(line, start);
}

std::string ErrorClass::error(int line, int pos, const std::string &msg,
                              const std::string &note,
                              const std::string &errorType,
                              const std::string &filename, Lexer &lexer,
                              const std::vector<Lexer::Token> &tokens,
                              bool isParser, bool isWarning, bool isFatal,
                              bool isMain, bool isTypeError, bool isGeneration) {
  std::string line_error =
      "[" + std::to_string(line) + "::" + std::to_string(pos) + "] (";
  if (isWarning) {
    line_error += col.color("Warning", Color::YELLOW, false, true);
  } else if (isFatal) {
    line_error += col.color("Fatal", Color::RED, false, true);
  } else {
    line_error += col.color("Error", Color::RED);
  }
  line_error += ") (" + filename + ")\n ↳ ";
  line_error +=
      (errorType.empty() ? col.color("Error", Color::RED, false, true)
                         : col.color(errorType, Color::MAGENTA, true, true)) +
      ": " + msg + "\n";

  if (isMain) {
    line_error +=
        "\t" + col.color("Note", Color::CYAN, false, true) + ": " + note + "\n";
    errors[line] = line_error;
    return line_error;
  }

  if (isTypeError) {
    if (!note.empty()) {
      line_error += " ↳ " + col.color("NOTE", Color::BLUE) + ": " + note + "\n";
      typeErros.push_back(line_error);
      return line_error;
    }
    line_error += (line > 1) ? currentLine(line - 1, 0, lexer, isParser,
                                           isTypeError, tokens)
                             : "";
    line_error += currentLine(line, pos, lexer, isParser, isTypeError, tokens);
    line_error +=
        currentLine(line + 1, 0, lexer, isParser, isTypeError, tokens);
    typeErros.push_back(line_error);
    return line_error;
  }

  if (isGeneration) {
    errors[line] = line_error;
    return line_error;
  }

  if (!note.empty()) {
    line_error += " ↳ " + note + "\n";
  }

  line_error +=
      currentLine(line, pos, lexer, isParser, isTypeError, tokens) + "\n";

  if (errors.find(line) == errors.end()) {
    errors[line] = line_error;
  }
  return line_error;
}

void ErrorClass::printError() {
  if (!errors.empty() || !typeErros.empty()) {
    if (!errors.empty()) {
      std::cout << "\r\033[2K"; // Clear the line and move to start -- overwrite previous garbage
      std::cout << "Total number of Errors: "
                << col.color(std::to_string(errors.size()), Color::RED, true,
                             false)
                << std::endl;
      for (const std::pair<int, std::string>& errorPair : errors) {
        std::cout << errorPair.second << std::endl;
      }
    }
    if (!typeErros.empty()) {
      std::cout << "\033[2K"; // Clear the line
      std::cout << "Total number of Type Errors: "
                << col.color(std::to_string(typeErros.size()), Color::RED, true,
                             false)
                << std::endl;
      for (const std::string &error : typeErros) {
        std::cout << error << std::endl;
      }
    }
    Exit(ExitValue::_ERROR);
  }
}
