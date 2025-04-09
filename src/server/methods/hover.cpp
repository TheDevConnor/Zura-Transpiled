#include "../../helper/math/math.hpp"
#include "../../parser/parser.hpp"
#include "../lsp.hpp"
#include "../logging.hpp"

#include <fstream>

nlohmann::ordered_json lsp::methods::hover(nlohmann::json& request) {
  // on-demand hover
  using namespace nlohmann;

  // hoverparams extends textdocumentpositionparams
  Position pos = { .line = request["params"]["position"]["line"], .character = request["params"]["position"]["character"] };
  std::string uri = request["params"]["textDocument"]["uri"];

  // get the text file at the uri
  Word text = lsp::document::wordUnderPos(uri, pos);

  if (text.word.rfind("@", 0) == 0) { // TODO: Check if inside a string or a comment (not real code)
    // check the integrity of the word (check to see if it's a real @ function)
    const static std::unordered_map<std::string, std::string> atFunctions = {
      {"output", "It can be used to display something to a file descriptor or stdout (fd of 1).\n\nExample:\n```zura\n@output(1, \"Hello, World!\"); # \"Hello, World!\" expected\n```"},
      {"input", "It can be used to take input from a file descriptor or stdin (fd of 0). It will read up to the inputted amount of bytes or until there is a line break.\n\nExample:\n```zura\nhave age: [256]char = [0];\n@output(1, \"What is your age? \");\n@input(0, age, 256);\n```"},
      {"template", "It can be used to define the function or struct that follows as generic (applies to many types).\n\nExample:\n```zura\n@template <typename T>;\nconst genericFunction := fn (arg1: T) {\n\t# ...\n};\n```"},
      {"cast", "It can be used to interpret between the builtin types. Possible conversions include:\n- str <=> *char\n- int <=> float\n- int <=> str\n\nExample:\n```zura\nhave x: int = 42;\nhave y: [24]char = [0];\ny = @cast<[]char>(x);\n```"},
      {"import", "It can be used to import the global symbols from another Zura file.\n\nExample:\n`foo.zu` ```zura\nconst globalFunc := fn () void { ... };\n```\n`bar.zu` ```zura\n@import \"foo.zu\";\nglobalFunc();\n```"},
      {"extern", "It can be used when handling external libraries that must be added to the linker. It defines a symbol or symbols that will resolved by the linker.\n\nExample:\n```zura\n@link \"libc\"; # Passed to linker\n@extern \"printf\"; # Look up this symbol\n\nconst main := fn () int {\n\t@call<printf>(\"Hello, World!\"); # Call previously revealed function\n};\n```"},
      {"link", "It can be used when handling external libraries that must be added to the linker. It is used to define external libraries to link to the executable.\nThis function assumes they are already installed on your system.\n\nExample:\n```zura\n@link \"libc\"; # Passed to linker\n@extern \"printf\"; # Look up this symbol\n\nconst main := fn () int {\n\t@call<printf>(\"Hello, World!\"); # Call previously revealed function\n};\n```"},
      {"call", "It can be used to call an external function that is linked externally. The function being called must be linked and extern'd before using it.\n\nExample:\n```zura\n@link \"libc\"; # Passed to linker\n@extern \"printf\"; # Look up this symbol\n\nconst main := fn () int {\n\t@call<printf>(\"Hello, World!\"); # Call previously revealed function\n};\n```"},
      {"alloc", "It can be used to allocate a certain number of bytes in memory. The only input required is the number of the bytes to allocate and returns the pointer to the allocated memory.\n\nExample:\n```zura\n@alloc(1024); # Allocates 1024 bytes of memory in your system\n```"},
      {"free", "It can be used to free previously allocated memory using @alloc. The only inputs are the pointer and the number of bytes to free and returns a status code, 0 for success and -1 for failure..\n\nExample:\n```zura\nhave ptr: *void = @alloc(1024);\n# ...\n@free(ptr, 1024); # frees the memory at ptr\n```"},
      {"sizeof", "It can be used to get the size of a type in bytes, calculated at comptime. It can be useful when trying to allocate a certain number of structs that you may change the size of later. The only input required is the type and returns the size of the type in bytes.\n\nExample:\n```zura\nhave x: int;\n@output(@sizeof(x)); # 8\n\nhave y: Test;\n@alloc(@sizeof(y)); # If you change the members of Y, this will still work as intended\n```"},
      {"memcpy", "It can be used to copy a certain number of bytes from one memory location to another. The only inputs required are the two pointers and the byte count. It is a void function.\n\nExample:\n```zura\nstruct Test {\n\tx: int;\n\ty: int;\n};\n\nhave x: Test = { x: 1, y: 2 };\nhave y: Test;\n@memcpy(&x, &y, @sizeof(x)); # Copy { 1, 2 } -> y\n```"},
      {"open", "It can be used to open a file located at a certain path. On success, it returns a positive integer representing the file descriptor of the opened file, otherwise a negative number is returned. It takes up to three arguments, specifying what can happen to the file: read, write, and create (if the file at the path does not exist).\n\nExample:\n```zura\nhave fd: int = @open(\"Testfile.txt\", true, false, false);\nif (fd < 0) {\n\t@output(1, \"Error opening file.\");\n\treturn -1;\n}\n@close(fd);```"},
      {"close", "It can be used to a close file opened by @open. All that is required is the file descriptor of the file.\n\nExample:\n```zura\nhave fd: int = @open(\"Testfile.txt\", true, false, false);\nif (fd < 0) {\n\t@output(1, \"Error opening file.\");\n\treturn -1;\n}\n@close(fd);```"},
    };
    std::unordered_map<std::string, std::string>::iterator closestMatch;
    int closestMatchDistance = 1000;
    std::string funcName = text.word.substr(1);
    if (atFunctions.find(funcName) == atFunctions.end()) {
      std::string closestMatch = "";
      size_t closestDst = 1000;
      for (auto& [key, value] : atFunctions) {
        size_t dst = levenshtein_distance(funcName, key);
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
          {"character", text.range.start.character + 1}
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
  } else {
    // A normal hover request LOL here we go!
    // Get the type of the word
    using namespace TypeChecker;
    // make the TC do something!!!
    LSPIdentifier ident = getIdentifierUnderPos(pos);
    if (ident.ident == "*") { // illegal character ensures you cannot trick us hahahah
      return nullptr; // Means nothing. You hovered over a space or something stupid
    }
    std::string type = type_to_string(ident.underlying);
    std::string markupResult;
    switch(ident.type) {
      case LSPIdentifierType::Struct:
        markupResult = "Struct `" + ident.ident + "`";
        break;
      case LSPIdentifierType::Enum:
        markupResult = "Enum `" + ident.ident + "`";
        break;
      case LSPIdentifierType::Function:
        markupResult = "Function `" + ident.ident + "`";
        break;
      case LSPIdentifierType::Variable:
        markupResult = "Variable `" + ident.ident + "`";
        break;
      default:
        markupResult = "Unknown `" + ident.ident + "`";
        break;
    }
    return {
      {"range", {
        {"start", {
          {"line", pos.line},
          {"character", text.range.start.character + 1} // It was always before for some reason. I dont know why
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
        {"value", markupResult}
      }}
    };
  }

  return nullptr;
}

TypeChecker::LSPIdentifier lsp::getIdentifierUnderPos(lsp::Position pos) {
  using namespace TypeChecker;
  if (lsp_idents.empty()) return LSPIdentifier { .underlying = nullptr, .type = LSPIdentifierType::Unknown, .ident = "*", .line = pos.line, .pos = pos.character };
  for (LSPIdentifier &ident : lsp_idents) {
    if ((ident.line - 1) == pos.line
        && pos.character >= ident.pos
        && pos.character < (ident.pos + ident.ident.size())) {
      return ident;
    }
  }
  return LSPIdentifier { .underlying = nullptr, .type = LSPIdentifierType::Unknown, .ident = "*", .line = pos.line, .pos = pos.character };
}