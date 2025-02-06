#include <fstream>
#include "../logging.hpp"
#include "../lsp.hpp"

nlohmann::ordered_json lsp::methods::completion(nlohmann::json& request) {
  using namespace nlohmann;

  json position = request["params"]["position"];

  if (request["params"]["context"]["triggerCharacter"] == "@" ||
    lsp::document::charAtPos(request["params"]["textDocument"]["uri"], Position {.line = position["line"], .character = int(position["character"]) - 1}) == '@') {
      return ordered_json {
        {"isIncomplete", false},
        {"items", ordered_json::array({
          {
            {"label", "call",},
            {"detail", "Call a linked function",},
            {"kind", 2,},
            {"documentation", "This native function pairs with the @link and @extern directives to call a native function from an externally linked library. The parameters will be appended in accordance with the ABI."}
          },
          {
            {"label", "link",},
            {"detail", "Link an external library",},
            {"kind", 2,},
            {"documentation", "This directive pairs with the @call and @extern directives to link a library to the outputted assembly, if the project is built into an executable. It is also useful for non-computer users of your code to understand what external libraries you are using. This function assumes that the library(s) in question are already installed on your system."}
          },
          {
            {"label", "extern",},
            {"detail", "Declare an external function, linked from an external library",},
            {"kind", 2,},
            {"documentation", "This directive pairs with the @call and @link directives to declare the use of a function that is implemented in an external library. This directive tells the linker that this symbol will be resolved by link-time, for each @call that happens to use it. This directive is useful for interacting with external libraries written in another ABI-compliant language, for binding purposes."}
          },
          {
            {"label", "template",},
            {"detail", "Declare a function as template for use with generic types.",},
            {"kind", 4,},
            {"documentation", "This directive tells the compiler that the function that follows is a template, and will be resolved to whatever types are using it at compile time. You can use template functions as a replacement to overloads if the code is similar enough. This is based heavily off of C++'s template functions. Read more at https://en.cppreference.com/w/cpp/language/function_template }."}
          },
          {
            {"label", "cast",},
            {"detail", "Statically cast a value to another type",},
            {"kind", 3,},
            {"documentation", "This function is used to cast a value to another type. This follows the same general footprint as casting in C-like languages, like `(int)4.0f`. Coercion can be done with unions, and structs must be casted manually}."}
          },
          {
            {"label", "import",},
            {"detail", "Import the global functions from another source file",},
            {"kind", 2,},
            {"documentation", "By declaring functions as public, they will be globally accessible to any other source file that imports them. The global namespace of the two files will be joined together in the outputted assembly."},
            {"insertText", "import \"\";"},
            {"insertTextFormat", 2}
          }
        })}
      };
  }
  // TODO: Get the TypeChecker to go sicko mode and do this for us
  return ordered_json {
    {"isIncomplete", false},
    {"items", ordered_json::array()} // explicitly empty array (there are no values)
  };
}