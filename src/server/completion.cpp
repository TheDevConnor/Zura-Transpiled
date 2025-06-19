#include "json.hpp"
#include "lsp.hpp"

namespace lsp {
  enum class InsertTextFormat {
    PlainText = 1,
    Snippet = 2
  };

  struct CompletionItem {
    std::string label;
    std::string documentation;
    std::string detail;
    std::string snippet;
    InsertTextFormat format;
  };

  inline std::vector<CompletionItem> globalCompletions = {
    // Types: int!, int?, char, str, bool, void, nil, float
    {"int!", "Non-nullable 64-bit signed integer.", "Type", "", InsertTextFormat::PlainText},
    {"int?", "Nullable 64-bit signed integer.", "Type", "", InsertTextFormat::PlainText},
    {"char", "Single character type.", "Type", "", InsertTextFormat::PlainText},
    {"str", "String type.", "Type", "", InsertTextFormat::PlainText},
    {"bool", "Boolean type.", "Type", "", InsertTextFormat::PlainText},
    {"void", "No value type, used for functions that do not return a value.", "Type", "", InsertTextFormat::PlainText},
    {"nil", "Null value, represents no value.", "Type", "", InsertTextFormat::PlainText},
    {"float", "Floating-point number type.", "Type", "", InsertTextFormat::PlainText},
    // Builtin Functions
    {"@cast", "Casts to another type.", "Builtin", "", InsertTextFormat::PlainText},
    // Keywords
    {"and", "Logical AND operator.", "", "", InsertTextFormat::PlainText},
    {"return", "Returns a value.", "", "return $0;", InsertTextFormat::Snippet},
    {"fn", "Defines a function.", "", "const $1 := fn($2) $3 {\n\t$0\n};", InsertTextFormat::Snippet},
    {"struct", "Defines a structured type.", "", "const $1 := struct {\n\t$0\n};", InsertTextFormat::Snippet},
    {"enum", "Defines an enumeration type.", "", "const $1 := enum {\n\t$0\n};", InsertTextFormat::Snippet},
    {"if", "Conditional statement.", "", "if ($1) {\n\t$0\n}", InsertTextFormat::Snippet},
    {"else", "Default case in an if-else chain.", "", "else {\n\t$0\n}", InsertTextFormat::Snippet},
    {"loop", "Loop with condition.", "", "loop ($1) {\n\t$0\n}", InsertTextFormat::Snippet},
  };
}

void lsp::handleMethodTextDocumentCompletion(const nlohmann::json& object) {
  const auto& params = object["params"];
  URI uri = params["textDocument"]["uri"];
  if (!documents.contains(uri)) return;

  nlohmann::json completions = nlohmann::json::array();

  for (const auto& item : globalCompletions) {
    logFile << "Adding completion item: " << item.label << "\n" << std::flush;
    nlohmann::json entry = {
      {"label", item.label},
      {"kind", 14}, // Variable-like, customize if needed
      {"documentation", item.documentation},
      {"insertText", item.snippet.empty() ? item.label : item.snippet},
    };

    if (!item.detail.empty())
      entry["detail"] = item.detail;

    if (!item.snippet.empty())
      entry["insertTextFormat"] = static_cast<int>(item.format); // 2 = Snippet

    completions.push_back(entry);
  }

  nlohmann::json response = {
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", {
      {"isIncomplete", false},
      {"items", completions}
    }}
  };

  handleResponse(response);
}
