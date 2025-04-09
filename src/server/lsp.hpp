#pragma once
#include <vector>
#include "json.hpp"
#include "../typeChecker/type.hpp"

namespace lsp {
    // -- TYPES --

  struct Position {
    size_t line;
    size_t character;
  };

  struct Range {
    Position start;
    Position end;
  };

  enum class TextDocumentSyncKind {
    None = 0,
    Full = 1,
    Incremental = 2
  };

  enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25,
  };

  struct Word {
    std::string word;
    Range range;
  };

  // -- METHODS --

  void initialize();
  void main();

  namespace methods {
    nlohmann::ordered_json initialize(nlohmann::json& request);
    nlohmann::ordered_json shutdown(nlohmann::json& request); // In this scenario, the return type nor the input is used- "null" is always returned
    nlohmann::ordered_json exit(nlohmann::json& request);     // Return type and parameter not used
    nlohmann::ordered_json completion(nlohmann::json& request);
    nlohmann::ordered_json cancelRequest(nlohmann::json& request); // We don't care about what is getting cancelled, but we must respond to these events anyway
    nlohmann::ordered_json hover(nlohmann::json& request);
  }

  namespace events {
    void documentOpen(nlohmann::json& request);
    void documentChange(nlohmann::json& request); // The complicated one
  };

  namespace document {
    void setCharAtPos(std::string uri, Position pos, std::string changeChar);
    char charAtPos(std::string uri, Position pos);
    Word wordUnderPos(std::string uri, Position pos);
    std::string getText(std::string uri);
  };

  TypeChecker::LSPIdentifier getIdentifierUnderPos(Position pos);

  std::vector<std::string> split(std::string in, std::string delim);
};