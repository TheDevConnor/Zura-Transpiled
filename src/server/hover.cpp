#include "lsp.hpp"
#include "../codegen/gen.hpp"
#include "../typeChecker/type.hpp"

// And completions!
bool lsp::isTokenCharacter(char c) {

  return std::isalnum(c) || c == '@' || c == '_';
}

char lsp::getCharUnder(const std::string &text, size_t line, size_t character) {
  // Returns the character at the given line and character position.
  size_t currentLine = 0;
  size_t currentChar = 0;
  size_t offset = 0;

  // Step 1: Find the byte offset at (line, character)
  while (offset < text.size() && currentLine < line) {
    if (text[offset] == '\n') {
      currentLine++;
      currentChar = 0;
    } else {
      currentChar++;
    }
    offset++;
  }

  // Now walk from the start of the line to the character offset
  while (offset < text.size() && text[offset] != '\n' && currentChar < character) {
    offset++;
    currentChar++;
  }

  if (offset >= text.size() || text[offset] == '\n') {
    return '\0'; // No character found
  }

  return text[offset];
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

size_t lsp::fileIDFromURI(lsp::URI uri) {
  if (!mainFileLink.contains(uri)) return (size_t)-1; // No main file link for this URI
  for (size_t i = 0; i < codegen::fileIDs.size(); i++) {
    // Trace an absolute path from the main file to this file id
    if (i >= codegen::fileIDs.size()) return (size_t)-1; // Out of bounds, no such file ID
    if ((std::filesystem::path(mainFileLink[uri].substr(7)) / codegen::fileIDs.at(i)).string() == uri.substr(7)) {
      // Yay! That was the one! We found it!
      return i;
    }
  }
  return (size_t)-1;
};


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
  // there is a lsp_idents vector that contains a bunch of, you guessed it, identifiers
  for (const auto &ident : TypeChecker::lsp_idents) {
    if (
      ident.ident == word.text && 
      ident.fileID == fileIDFromURI(object["params"]["textDocument"]["uri"]) &&
      (ident.line - 1) == word.range.start.line) {
      nlohmann::json response = {
        {"jsonrpc", "2.0"},
        {"id", object["id"]},
        {"result", {
          {"contents", {
            {"kind", "markdown"},
            {"value", "```zura\n" + ident.ident + ": " + TypeChecker::type_to_string(ident.underlying) + "\n```"}
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
  }
  handleResponse({
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", nullptr} // No identifier found
  });
  return;
}