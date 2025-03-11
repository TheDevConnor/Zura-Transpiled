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
  } else if (request["params"]["context"]["triggerCharacter"] == "." ||
    lsp::document::charAtPos(request["params"]["textDocument"]["uri"], Position {.line = position["line"], .character = size_t(position["character"]) - 1}) == '.') {
    // Triggered by pressing a dot
    // Get the word before the dot
    std::string documentContent = lsp::document::getText(request["params"]["textDocument"]["uri"]);
    // Ask typechecker for the type of this identifier
    // by getting the identifier under this position
    TypeChecker::LSPIdentifier ident = getIdentifierUnderPos(Position {.line = position["line"], .character = (size_t)position["character"] - 1});
    logging::log("Ident: " + ident.ident + "," + TypeChecker::type_to_string(ident.underlying) + "\n");
    // If the identifier is a struct, enum, or function, we can provide completions
    // if (ident.type == TypeChecker::LSPIdentifierType::Struct) {
    //   // Check struct table for members
    //   ordered_json completions = ordered_json::array(); // { label, kind, detail, documentation }
    //   for (auto &field : TypeChecker::map.get()->struct_table[ident.ident]) {
    //     completions.push_back({
    //       {"label", field.first},
    //       {"kind", 5}, // Field
    //       {"detail", "Field " + field.first},
    //       {"labelDetails", {
    //         {"detail", ": " + TypeChecker::type_to_string(field.second)}
    //       }}
    //     });
    //   }
    //   // Member functions (methods)
    //   for (auto &fn : TypeChecker::map.get()->struct_table_fn[ident.ident]) {
    //     completions.push_back({
    //       {"label", fn.first.first},
    //       {"kind", 2}, // Method
    //       {"detail", "Function " + fn.first.first},
    //       {"labelDetails", {
    //         {"detail", ": " + TypeChecker::type_to_string(fn.first.second)}
    //       }}
    //     });
    //   }
    //   return ordered_json {
    //     {"isIncomplete", false},
    //     {"items", completions}
    //   };
    // } else if (ident.type == TypeChecker::LSPIdentifierType::Enum) {
    //   // Possibly the easiest one to implement, as each field is just an Enumerator
    //   ordered_json completions = ordered_json::array();
    //   for (auto &field : TypeChecker::map.get()->enum_table[ident.ident]) {
    //     completions.push_back({
    //       {"label", field.first},
    //       {"kind", 20}, // EnumMember
    //       {"detail", "Enumerator " + field.first},
    //       {"labelDetails", {
    //         {"detail", ": " + std::to_string(field.second)} // int - the enum value
    //       }}
    //     });
    //   }
    //   return ordered_json {
    //     {"isIncomplete", false},
    //     {"items", completions}
    //   };
    // } else if (ident.type == TypeChecker::LSPIdentifierType::Function) {
    //   // Check function table for parameters
    //   ordered_json completions = ordered_json::array();
    //   for (auto &fn : TypeChecker::map.get()->function_table) {
    //     if (fn.first.first == ident.ident) {
    //       std::string parameters = "";
    //       for (size_t i = 0; i < fn.second.size(); i++) {
    //         std::pair<std::string, Node::Type *> &param = fn.second[i];
    //         parameters += param.first + ": " + TypeChecker::type_to_string(param.second);
    //         // append comma if not the last element
    //         if (i != fn.second.size() - 1) parameters += ", ";
    //       }
    //       completions.push_back({
    //         {"label", fn.first.first},
    //         {"kind", 3}, // Function
    //         {"detail", "Function " + fn.first.first},
    //         {"labelDetails", {
    //           {"detail", "(" + parameters + "): " + TypeChecker::type_to_string(fn.first.second)}
    //         }}
    //       });
    //     }
    //   }
    //   return ordered_json {
    //     {"isIncomplete", false},
    //     {"items", completions}
    //   };
    // }
  }
  return ordered_json {
    {"isIncomplete", false},
    {"items", ordered_json::array()} // explicitly empty array (there are no values)
  };
}