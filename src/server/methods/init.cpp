#include "../json.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::initialize(nlohmann::json& request) {
  using namespace nlohmann;
  // check if the client supports the capabilities that WE provide
  // const bool supportsCompletions = true;
  // const bool supportsHover = true;
  // const bool supportsTextSync = true;

  // literally broken lol
  const bool supportsCompletions = request.contains("completionProvider");
  const bool supportsHover = request.contains("hoverProvider");
  const bool supportsTextSync = request.contains("textDocumentSync");

  ordered_json response = {
    {"capabilities", {
      {"textDocumentSync", supportsTextSync ? TextDocumentSyncKind::Incremental : TextDocumentSyncKind::None},
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