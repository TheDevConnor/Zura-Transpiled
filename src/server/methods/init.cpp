#include "../json.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::initialize(nlohmann::json& request) {
  using namespace nlohmann;
  // check if the client supports the capabilities that WE provide
  bool supportsCompletions = true;
  bool supportsHover = true;
  bool supportsTextSync = true;

  supportsCompletions = request.contains("completionProvider");
  supportsHover = request.contains("hoverProvider");
  supportsTextSync = request.contains("textDocumentSync");

  ordered_json response = {
    {"capabilities", {
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
  if (supportsHover) {
    response["capabilities"]["hoverProvider"] = true;
  }
  if (supportsTextSync) {
    response["capabilities"]["textDocumentSync"] = {
      TextDocumentSyncKind::Incremental
    };
  }
  return response;
}