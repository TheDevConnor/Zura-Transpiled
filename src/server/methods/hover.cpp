#include "../lsp.hpp"
#include "../../helper/math/math.hpp"
#include <fstream>

nlohmann::ordered_json lsp::methods::hover(nlohmann::json& request) {
  // on-demand hover
  using namespace nlohmann;

  // hoverparams extends textdocumentpositionparams
  Position pos = { .line = request["params"]["position"]["line"], .character = request["params"]["position"]["character"] };
  std::string uri = request["params"]["textDocument"]["uri"];

  // get the text file at the uri
  Word text = lsp::document::wordUnderPos(uri, pos);

  if (text.word.starts_with("@")) {
    // check the integrity of the word (check to see if it's a real @ function)
    // see if the atFunctions.json file contains the word
    std::ifstream file("src/server/atFunctions.json");
    bool hasFunction = false;
    const static std::vector<std::string> atFunctions = {
      "output",
      "input",
      "template",
      "call",
      "cast",
      "import",
      "extern",
      "link",
      "call"
    };
    std::string closestMatch = "";
    int closestMatchDistance = 1000;
    for (int i = 0; i < atFunctions.size(); i++) {
      std::string functionName = atFunctions[i];
      if (functionName == text.word.substr(1)) {
        hasFunction = true;
        break;
      }
      int distance = levenshtein_distance(functionName, text.word.substr(1));
      if (distance <= 3 && distance < closestMatchDistance) {
        closestMatch = functionName;
        closestMatchDistance = distance;
      }
    }
    if (!hasFunction) {
      std::string result = closestMatchDistance <= 3 ? " Did you mean `@" + closestMatch + "`?" : "";
      return {
        {"range", {
          {"start", {
            {"line", pos.line},
            {"character", text.range.start.character}
          }},
          {"end", {
            {"line", pos.line},
            {"character", text.range.end.character}
          }}
        }},
        // scold you for using an unknown function
        {"contents", {
          {"kind", "markdown"},
          {"value", "### Unknown built-in function `" + text.word + "`\n\nThis built-in function does not exist in Zura." + result}
        }}
      };
    }
    // it was a builtin @ function
    return {
      {"range", {
        {"start", {
          {"line", pos.line},
          {"character", text.range.start.character}
        }},
        {"end", {
          {"line", pos.line},
          {"character", text.range.end.character}
        }}
      }},
      // the actual hover text
      // title
      {"contents", {
        {"kind", "markdown"},
        {"value", "## Built-in function `" + text.word + "`\n\nThis is a built-in function provided by Zura.\nNo external libraries are required in order to access this function."}
      }}
    };
  }

  return nullptr;
}