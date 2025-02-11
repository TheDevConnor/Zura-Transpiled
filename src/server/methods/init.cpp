#include "../json.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::initialize(nlohmann::json& request) {
  using namespace nlohmann;
  // check if the client supports the capabilities that WE provide
  const bool supportsCompletions = true;
  const bool supportsHover = true;
  const bool supportsTextSync = true;

  // literally broken lol
  // supportsCompletions = request.contains("completionProvider");
  // supportsHover = request.contains("hoverProvider");
  // supportsTextSync = request.contains("textDocumentSync");

  ordered_json response = {
    {"capabilities", {
      {"completionProvider", {
        {"resolveProvider", false},
        {"triggerCharacters", json::array({"@", "."})},
        {"completionItem", {
          {"snippetSupport", true}
        }}
      }},
      {"textDocumentSync", TextDocumentSyncKind::Incremental},
      {"hoverProvider", true}
    }},
    {"serverInfo", {
      {"name", "zura-lsp"},
      {"version", "0.0.1"}
    }}
  };
  return response;
}