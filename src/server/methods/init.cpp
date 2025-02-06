#include "../json.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::initialize(nlohmann::json& request) {
  using namespace nlohmann;
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