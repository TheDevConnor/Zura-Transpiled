#include <fstream>
#include "../logging.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::completion(nlohmann::json& request) {
  using namespace nlohmann;

  json position = request["params"]["position"];

  if (request["params"]["context"]["triggerCharacter"] == "@" ||
    lsp::document::charAtPos(request["params"]["textDocument"]["uri"], Position {.line = position["line"], .character = size_t(position["character"]) - 1}) == '@') {
      return ordered_json {
        {"isIncomplete", false},
        {"items", ordered_json::parse(std::ifstream("src/server/atFunctions.json"))}
      };
  }
  // TODO: Get the TypeChecker to go sicko mode and do this for us
  return ordered_json {
    {"isIncomplete", false},
    {"items", ordered_json::array()} // explicitly empty array (there are no values)
  };
}