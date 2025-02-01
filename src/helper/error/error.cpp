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
                                 bool highlightPos = false, bool getLastToken) {
  std::string formattedLine = lineNumber(line) + std::to_string(line) + " | ";
  const Lexer::Token *lastToken = nullptr;
  int errorPos = formattedLine.size();
  ErrorPos = 0;

  for (const Lexer::Token &tk : tokens) {
    if (tk.line != line) continue;

    if (highlightPos && tk.column == pos) {
      if (getLastToken && lastToken) {
        formattedLine += col.color(lastToken->value, Color::RED, false, true) + " ";
        errorPos += errorPos + lastToken->value.size() + 1;
      } else {
        formattedLine += col.color("_", Color::RED, false, true) + col.color(tk.value, Color::RED, false, true) + " ";
        ErrorPos = errorPos; 
      }
    } else {
      formattedLine += tk.value + " ";
    }

    lastToken = &tk;
    errorPos += tk.value.size() + 1;
  }

  formattedLine += "\n";
  return formattedLine;
}

std::string ErrorClass::currentLine(int line, int pos, Lexer &lexer,
                                    bool isParser, bool isTypeError,
                                    const std::vector<Lexer::Token> &tokens, bool getLastToken) {
  if (isParser || isTypeError || getLastToken) {
    return formatLineWithTokens(line, pos, tokens, isParser, getLastToken);
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

  std::string line_error = "[" + std::to_string(line) + "::" + std::to_string(pos) + "] (";

  if (isWarning) {
    line_error += col.color("Warning", Color::YELLOW, false, true);
  } else if (isFatal) {
    line_error += col.color("Fatal", Color::RED, false, true);
  } else {
    line_error += col.color("Error", Color::RED);
  }

  line_error += ") (" + filename + ")\n ↳ ";
  line_error += (errorType.empty() ? col.color("Error", Color::RED, false, true)
                                   : col.color(errorType, Color::MAGENTA, true, true)) + ": ";
  // check if the msg begins with 'No value found for key'
  if (msg.find("No value found for key") != std::string::npos) {
    // check if fatal and error
    if (isFatal) {
      printError();
      Exit(ExitValue::_ERROR);
    }
    return line_error; // no need to print the message
  } else {
    line_error += msg + "\n";
  }

  if (isMain) {
    line_error += "\t" + col.color("Note", Color::CYAN, false, true) + ": " + note + "\n";
    errors[line] = line_error;
    return line_error;
  }

  if (isParser) {
    if (msg.find("Expected a SEMICOLON") == 0) {
        line_error += currentLine(line - 1, pos, lexer, isParser, isTypeError, tokens, true);
        line_error += " ↳ " + col.color("NOTE", Color::BLUE) + ": If the line above is empty, it's possible that the error is on the line above.\n";
        if (!note.empty()) line_error += " ↳ " + col.color("NOTE", Color::BLUE) + ": " + note + "\n";
    } else {
        line_error += currentLine(line, pos, lexer, isParser, isTypeError, tokens);
        line_error += std::string(ErrorPos, ' ') + col.color("^", Color::RED, false, true) + "\n";
        if (!note.empty()) line_error += " ↳ " + col.color("NOTE", Color::BLUE) + ": " + note + "\n";
    }

    if (errors.find(line) == errors.end()) {
        errors[line] = line_error; // Add error if not already present
    }
    return line_error;
  }

  if (isTypeError) {
    if (!note.empty()) {
      line_error += " ↳ " + col.color("NOTE", Color::BLUE) + ": " + note + "\n";
      errors[line] = line_error;
      return line_error;
    }

    line_error += (line > 0) ? currentLine(line - 1, pos, lexer, isParser, isTypeError, tokens) : "";
    line_error += col.color(currentLine(line, pos, lexer, isParser, isTypeError, tokens), Color::GRAY, false, true);
    line_error += currentLine(line + 1, pos, lexer, isParser, isTypeError, tokens);
    errors[line] = line_error;
    return line_error;
  }

  if (isGeneration) {
    if (!note.empty()) {
      line_error += " ↳ " + col.color("NOTE", Color::BLUE) + ": " + note + "\n";
    }
    line_error += currentLine(line, pos, lexer, isParser, isTypeError, tokens);
    line_error += std::string(pos + 5, ' ') + col.color("^", Color::RED, false, true) + "\n";
    errors[line] = line_error;
    return line_error;
  }

  if (!note.empty()) line_error += " ↳ " + note + "\n";

  // if we get here, it's a lexer error
  line_error += currentLine(line, pos, lexer, isParser, isTypeError, tokens);
  line_error += std::string(pos + 5, ' ') + col.color("^", Color::RED, false, true) + "\n";

  if (errors.find(line) == errors.end()) errors[line] = line_error;
  if (isFatal) {
    std::cout << "Guys its fatal bitch" << std::endl;
    printError();
    if (isGeneration) Exit(ExitValue::GENERATOR_ERROR);
    if (isTypeError) Exit(ExitValue::TYPE_ERROR);
    if (isParser) Exit(ExitValue::PARSER_ERROR); 
    Exit(ExitValue::LEXER_ERROR); // Process of elimination
  }
  return line_error;
}

void ErrorClass::printError() {
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
}
