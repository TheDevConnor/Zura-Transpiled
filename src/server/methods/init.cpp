#include "../json.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::initialize(nlohmann::json& request) {
  using namespace nlohmann;
  // check if the client supports the capabilities that WE provide
  
  // literally broken lol
  bool supportsCompletions = request.contains("completionProvider");
  bool supportsHover = request.contains("hoverProvider");
  bool supportsTextSync = request.contains("textDocumentSync");
  supportsCompletions = true;
  supportsHover = true;
  supportsTextSync = true;

  ordered_json response = {
    {"capabilities", {
      {"textDocumentSync", supportsTextSync ? TextDocumentSyncKind::Incremental : TextDocumentSyncKind::Full},
      {"hoverProvider", supportsHover}
    }},
    {"serverInfo", {
      {"name", "zura-lsp"},
      {"version", "0.0.1"}
    }}
  };
  if (supportsCompletions) {
    response["capabilities"]["completionProvider"] = {
      {"resolveProvider", false},
      {"triggerCharacters", json::array({"@", "."})},
      {"completionItem", {
        {"snippetSupport", true}
      }}
    };
  }
  return response;
}