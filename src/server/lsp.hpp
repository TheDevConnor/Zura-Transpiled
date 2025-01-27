#include <vector>
#include "json.hpp"

namespace lsp {
    // -- TYPES --

  struct Position {
    int line;
    int character;
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

  // -- METHODS --

  void initialize();
  void main();

  namespace methods {
    nlohmann::ordered_json initialize(nlohmann::json& request);
    nlohmann::ordered_json shutdown(nlohmann::json& request); // In this scenario, the return type nor the input is used- "null" is always returned
    nlohmann::ordered_json exit(nlohmann::json& request);     // Return type and parameter not used
    nlohmann::ordered_json completion(nlohmann::json& request);
    nlohmann::ordered_json cancelRequest(nlohmann::json& request); // We don't care about what is getting cancelled, but we must respond to these events anyway
  }

  namespace events {
    void documentOpen(nlohmann::json& request);
    void documentChange(nlohmann::json& request); // The complicated one
  };

  namespace document {
    void setCharAtPos(std::string uri, Position pos, char changeChar);
    char charAtPos(std::string uri, Position pos);
  };

  std::vector<std::string> split(std::string in, std::string delim);
};