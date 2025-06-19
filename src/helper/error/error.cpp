#include "error.hpp"

#include <iostream>
#include <string>

#include "../../common.hpp"
#include "../term_color/color.hpp"

Color col;

std::string Error::error_head(std::string error_type, int line, int pos,
                              std::string filepath, bool isWarn) {
  std::string ln = col.color(std::to_string(line), Color::C::YELLOW, true, false);
  std::string ps = col.color(std::to_string(pos), Color::C::YELLOW, true, false);
  std::string error = "";
  if (isWarn) {
    error = col.color("warn", Color::C::YELLOW, false, true) + ": ";
  } else {
    error = col.color("error", Color::C::RED, false, true) + ": ";
  }
  std::string type = col.color(error_type, Color::C::WHITE, true, true);
  return error + type + "\n  --> [" + ln + "::" + ps + "](" + filepath + ")\n";
}

std::string Error::generate_whitespace(int space) {
  std::string final;
  for (int i = 0; i < space; i++)
    final += " ";
  return final;
}

std::string Error::generate_line(const std::vector<Lexer::Token> &tks, int line, int pos, Color::C c) {
  (void)pos;
  std::string ln = "";

  // Safety check to handle empty token list
  if (tks.empty()) {
    return ln + "\n";
  }

  try {
    for (const Lexer::Token &tk : tks) {
      if (tk.line != line) continue;
      ln += col.color(generate_whitespace(tk.whitespace) + tk.value, c, false, true);
    }
    ln += "\n";
    return ln;
  } catch (const std::exception &e) {
    // If any exception occurs while generating the line, return a safe default
    return "Error generating line: " + std::string(e.what()) + "\n";
  }
}

bool Error::report_error() {
  if (!shouldPrintErrors) return errors.size() > 0;
  if (errors.size() > 0) {
    std::cout << "Total Errors: "
              << col.color(std::to_string(errors.size()), Color::C::RED) << "\n";
    for (ErrorInfo error : errors)
      std::cout << error.message << std::endl;
    return true;
  }
  if (warnings.size() > 0) {
    std::cout << "Total Warnigns: "
              << col.color(std::to_string(warnings.size()), Color::C::YELLOW) << "\n";
    for (ErrorInfo warning : warnings)
      std::cout << warning.message << std::endl;
    return false;
  }
  return false;
}

void Error::handle_lexer_error(Lexer &lex, std::string error_type,
                               std::string file_path, std::string msg) {
  try {
    const char *start = lex.lineStart(lex.scanner.line);
    int line_start = lex.scanner.line, col_start = lex.scanner.column;
    const char *end = start;
    while (*end != '\n' && *end != '\0')
      end++;

    std::string error = error_head(error_type, lex.scanner.line, lex.scanner.column, file_path, false);
    error += col.color("   |\n", Color::C::GRAY);
    std::string formatted_line =
        line_number(lex.scanner.line) + std::to_string(lex.scanner.line) + "|";
    error +=
        " " + formatted_line + std::string(start, unsigned(end - start)) + "\n";
    std::string error_space = std::string(static_cast<std::size_t>(std::max(0, lex.scanner.column - 1)), ' ');
    error += col.color("   |", Color::C::GRAY) + error_space + col.color("^", Color::C::RED, true, true) + "\n";
    error += col.color("note", Color::C::CYAN) + ": " + msg;
    errors.push_back(ErrorInfo {
                                .line_start = (unsigned long)line_start,
                                .col_start = (unsigned long)col_start,
                                .line_end = (unsigned long)lex.scanner.line,
                                .col_end = (unsigned long)lex.scanner.column,
                                .message = error, .simplified_message = msg,
                                .file_path = file_path,
    });
  } catch (const std::exception &e) {
    // If error formatting fails, make sure we at least report something
    std::string simpleError = "Error in " + file_path + " at line " +
                              std::to_string(lex.scanner.line) + ", pos " +
                              std::to_string(lex.scanner.column) + ": " + msg +
                              " (Error formatting failed: " + e.what() + ")";
    errors.push_back(ErrorInfo { .line_start = (unsigned long)lex.scanner.line,
                                 .col_start = (unsigned long)lex.scanner.column,
                                 .line_end = (unsigned long)lex.scanner.line,
                                 .col_end = (unsigned long)lex.scanner.column,
                                 .message = simpleError, .simplified_message = msg,
                                 .file_path = file_path });
  }
}

std::string Error::handle_type_error(const std::vector<Lexer::Token> &tks, int line,
                                 int pos) {
  std::string error;
  
  std::string formatted_line_before = line_number(line-1) + std::to_string(line-1) + "|";
  std::string formatted_line = line_number(line) + std::to_string(line) + "|";
  std::string formatted_line_after = line_number(line+1) + std::to_string(line+1) + "|";
  
  error += " " + formatted_line_before + generate_line(tks, line - 1, pos, Color::C::GRAY);
  error += " " + formatted_line + generate_line(tks, line, pos);
  error += col.color("   |", Color::C::GRAY) + generate_whitespace(pos - 1) +
           col.color("^", Color::C::RED, true, true) + "\n";
  error += " " + formatted_line_after + generate_line(tks, line + 1, pos, Color::C::GRAY);
  return error;
}

void Error::handle_error(std::string error_type, std::string file_path,
                         std::string msg, const std::vector<Lexer::Token> &tks,
                         int line, int pos, int endPos, bool isWarn) {
  if (msg.find("Expected a SEMICOLON") == 0) line = line - 1;
  try {
    ErrorInfo error = {
      .line_start = (unsigned long)line,
      .col_start = (unsigned long)pos,
      .line_end = (unsigned long)line,
      .col_end = (unsigned long)endPos, // TODO: Adjust this based on the actual error
      .message = "",
      .simplified_message = msg,
      .file_path = file_path,
    };
    error.simplified_message = msg;
    error.message += error_head(error_type, line, pos, file_path, isWarn);
    error.message += col.color("   |\n", Color::C::GRAY);
    std::string formatted_line = line_number(line) + std::to_string(line) + "|";

    // Handle the case when we're at the end of the file
    if (tks.empty()) {
      error.message += " " + formatted_line + "\n";
    } else if (error_type == "Type Error") {
      error.message += handle_type_error(tks, line, pos);
      error.message += col.color("note", Color::C::CYAN) + ": " + msg;
      errors.push_back(error);
      return;
    } else {
      error.message += " " + formatted_line + generate_line(tks, line, pos);
    }

    // Make sure we don't generate negative spaces
    int pointer_pos = pos > 0 ? pos - 1 : 0;
    std::string error_space = std::string(static_cast<std::size_t>(std::max(0, pointer_pos)), '~');
    error.message += col.color("   |", Color::C::GRAY) + col.color(error_space, Color::C::RED) + col.color("^", Color::C::RED, true, true) + "\n";
    error.message += col.color("note", Color::C::CYAN) + ": " + msg;
    if (isWarn)
      warnings.push_back(error);
    else
      errors.push_back(error);
  } catch (const std::exception &e) {
    // If any exception occurs while formatting the error, fallback to a simple message
    ErrorInfo simpleError = {
      .line_start = (unsigned long)line,
      .col_start = (unsigned long)pos,
      .line_end = (unsigned long)line,
      .col_end = (unsigned long)endPos, // TODO
      .message = "Error in " + file_path + " at line " +
      std::to_string(line) + ", pos " +
      std::to_string(pos) + ": " + msg +
      " (Error formatting failed: " + e.what() + ")",
      .simplified_message = msg,
      .file_path = file_path,
    };
    if (isWarn)
      warnings.push_back(simpleError);
    else
      errors.push_back(simpleError);
  }
}