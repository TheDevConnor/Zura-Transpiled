#pragma once

#include <string>
#include <vector>

#include "../term_color/color.hpp"
#include "../../lexer/lexer.hpp"

/*
 * Total errors: 1
 * error: TYPE_OF_ERROR
 *   --> [1::10](main.x)
 *    |
 *  1 | 5 + 7 / 0
 *    |          ^
 * note: ERROR_MSG
 */

class Error {
 public:
  struct ErrorInfo {
    unsigned long int line_start;
    unsigned long int col_start;
    unsigned long int line_end;
    unsigned long int col_end;
    std::string message;
    std::string simplified_message;
    std::string file_path;
  };

  inline static std::vector<ErrorInfo> errors = {};
  inline static std::vector<ErrorInfo> warnings = {};
  static void handle_lexer_error(Lexer &lex, std::string error_type,
                                 std::string file_path, std::string msg);
  static std::string handle_type_error(const std::vector<Lexer::Token> &tks, int line,
  int pos);
  static void handle_error(std::string error_type, std::string file_path,
                           std::string msg, const std::vector<Lexer::Token> &tks, int line, int pos, int endPos, bool isWarn = false);
  static bool report_error();

 private:
  static std::string error_head(std::string error_type, int line, int pos,
                                std::string filepath, bool isWarn);

  static std::string line_number(int line) { return (line < 10) ? "0" : ""; }
  static std::string generate_whitespace(int space);
  static std::string generate_line(const std::vector<Lexer::Token> &tks, int line, int pos, Color::C c = Color::C::WHITE);
};