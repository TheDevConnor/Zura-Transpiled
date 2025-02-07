#include "../../helper/math/math.hpp"
#include "../lsp.hpp"

#include <fstream>

nlohmann::ordered_json lsp::methods::hover(nlohmann::json& request) {
  // on-demand hover
  using namespace nlohmann;

  // hoverparams extends textdocumentpositionparams
  Position pos = { .line = request["params"]["position"]["line"], .character = request["params"]["position"]["character"] };
  std::string uri = request["params"]["textDocument"]["uri"];

  // get the text file at the uri
  Word text = lsp::document::wordUnderPos(uri, pos);

  if (text.word.starts_with("@")) { // TODO: Check if inside a string or a comment (not real code)
    // check the integrity of the word (check to see if it's a real @ function)
    const static std::unordered_map<std::string, std::string> atFunctions = {
      {"output", "It can be used to display something to the console (stdout).\n\nExample:\n```zura\n@output(\"Hello, World!\"); # \"Hello, World!\" expected\n```"},
      {"input", "It can be used to take input from the console (stdin). It will read up to the inputted amount of bytes or until there is a line break.\n\nExample:\n```zura\nhave age: [256]char = [0];\n@output(\"What is your age? \");\n@input(age, 256);\n```"},
      {"template", "It can be used to define the function or struct that follows as generic (applies to many types).\n\nExample:\n```zura\n@template <typename T>;\nconst genericFunction := fn (arg1: T) {\n\t# ...\n};\n```"},
      {"cast", "It can be used to interpret between the builtin types. Possible conversions include:\n- str <=> *char\n- int <=> float\n- int <=> str\n\nExample:\n```zura\nhave x: int = 42;\nhave y: [24]char = [0];\ny = @cast<[]char>(x);\n```"},
      {"import", "It can be used to import the global symbols from another Zura file.\n\nExample:\n`foo.zu` ```zura\nconst globalFunc := fn () void { ... };\n```\n`bar.zu` ```zura\n@import \"foo.zu\";\nglobalFunc();\n```"},
      {"extern", "It can be used when handling external libraries that must be added to the linker. It defines a symbol or symbols that will resolved by the linker.\n\nExample:\n```zura\n@link \"libc\"; # Passed to linker\n@extern \"printf\"; # Look up this symbol\n\nconst main := fn () int {\n\t@call<printf>(\"Hello, World!\"); # Call previously revealed function\n};\n```"},
      {"link", "It can be used when handling external libraries that must be added to the linker. It is used to define external libraries to link to the executable.\nThis function assumes they are already installed on your system.\n\nExample:\n```zura\n@link \"libc\"; # Passed to linker\n@extern \"printf\"; # Look up this symbol\n\nconst main := fn () int {\n\t@call<printf>(\"Hello, World!\"); # Call previously revealed function\n};\n```"},
      {"call", "It can be used to call an external function that is linked externally. The function being called must be linked and extern'd before using it.\n\nExample:\n```zura\n@link \"libc\"; # Passed to linker\n@extern \"printf\"; # Look up this symbol\n\nconst main := fn () int {\n\t@call<printf>(\"Hello, World!\"); # Call previously revealed function\n};\n```"},
      {"alloc", "It can be used to allocate a certain number of bytes in memory. The only input required is the number of the bytes to allocate and returns the pointer to the allocated memory.\n\nExample:\n```zura\n@alloc(1024); # Allocates 1024 bytes of memory in your system\n```"},
      {"free", "It can be used to free previously allocated memory using @alloc. The only inputs are the pointer and the number of bytes to free and returns a status code, 0 for success and -1 for failure..\n\nExample:\n```zura\nhave ptr: *void = @alloc(1024);\n# ...\n@free(ptr, 1024); # frees the memory at ptr\n```"},
    };
    std::unordered_map<std::string, std::string>::iterator closestMatch;
    int closestMatchDistance = 1000;
    std::string funcName = text.word.substr(1);
    if (atFunctions.find(funcName) == atFunctions.end()) {
      std::string closestMatch = "";
      int closestDst = 1000;
      for (auto& [key, value] : atFunctions) {
        int dst = levenshtein_distance(funcName, key);
        if (dst < closestDst) {
          closestDst = dst;
          closestMatch = key;
        }
      }

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
        // scold you for using an unknown function (imagine mispelling something, ahhahaha
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
        {"value", "## Built-in function `" + text.word + "`\n\nThis is a built-in function provided by Zura.\nNo external libraries are required in order to access this function.\n" + atFunctions.at(funcName)}
      }}
    };
  }

  return nullptr;
}