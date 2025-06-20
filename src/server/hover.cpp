#include "lsp.hpp"

// And completions!
bool lsp::isTokenCharacter(char c) {

  return std::isalnum(c) || c == '@' || c == '_';
}

lsp::Word lsp::getWordUnder(const std::string &text, size_t line, size_t character) {
  // Returns the closest collection of alphanumeric characters, at (@) signs, and underscores (_)
  // at a given position.
  size_t currentLine = 0;
  size_t currentChar = 0;
  size_t offset = 0;

  size_t lineStartOffset = 0;

  // Step 1: Find the byte offset at (line, character)
  while (offset < text.size() && currentLine < line) {
    if (text[offset] == '\n') {
      currentLine++;
      currentChar = 0;
      lineStartOffset = offset + 1;
    } else {
      currentChar++;
    }
    offset++;
  }

  // Now walk from lineStartOffset to the character offset
  size_t pos = lineStartOffset;
  size_t charInLine = 0;
  while (pos < text.size() && text[pos] != '\n' && charInLine < character) {
    pos++;
    charInLine++;
  }

  if (pos >= text.size() || !isTokenCharacter(text[pos])) {
    return { "", {{line, character}, {line, character}} };
  }

  // Step 2: Expand left
  size_t startOffset = pos;
  while (startOffset > lineStartOffset && isTokenCharacter(text[startOffset - 1])) {
    startOffset--;
  }
  
  // Step 3: Expand right
  size_t endOffset = pos;
  while (endOffset < text.size() && isTokenCharacter(text[endOffset])) {
    endOffset++;
  }

  // Step 4: Compute start and end positions (line and character)
  Position startPos = { line, 0 };
  for (size_t i = lineStartOffset; i < startOffset; ++i) {
    if (text[i] == '\t') startPos.character += 4; // Optional tab handling
    else startPos.character++;
  }

  Position endPos = { line, startPos.character };
  for (size_t i = startOffset; i < endOffset; ++i) {
    if (text[i] == '\t') endPos.character += 4;
    else endPos.character++;
  }

  std::string finalText = text.substr(startOffset, endOffset - startOffset);
  return { finalText, {startPos, endPos} };
}


void lsp::handleMethodTextDocumentHover(const nlohmann::json &object)
{
  // This method will be split into two parts:
  // 1. Checks for internal references, primarily builtin @ functions and types
  // 2. Checks for references within the actual codebase, like user-defined functions & variables

  //! 1. Check for internal references
  Word word = getWordUnder(documents[object["params"]["textDocument"]["uri"]], object["params"]["position"]["line"], object["params"]["position"]["character"]);
  if (word.text.starts_with("@")) {
    // This is a builtin function.
    std::string note = getBuiltinNote(word.text);
    if (note.empty()) {
      // No note found, return null
      nlohmann::json response = {
        {"jsonrpc", "2.0"},
        {"id", object["id"]},
        {"result", nullptr}
      };
      handleResponse(response);
      return;
    }
    nlohmann::json response = {
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", {
        {"contents", {
          {"kind", "markdown"},
          {"value", "# Builtin function '" + word.text + "'\n" + getBuiltinNote(word.text) + "\nThis builtin function is automatically provided to you by Zura."}
        }},
        {"range", {
          {"start", {{"line", word.range.start.line}, {"character", word.range.start.character}}},
          {"end", {{"line", word.range.end.line}, {"character", word.range.end.character}}}
        }}
      }}
    };
    handleResponse(response);
    return;
  }

  //! 2. Check for references within the actual codebase
  // We will not handle this. I'm FAR too lazy.
  nlohmann::json response = {
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", nullptr} // No hover information found
  };
  handleResponse(response);
  return;
}