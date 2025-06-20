#include "json.hpp"
#include "lsp.hpp"
#include "../typeChecker/type.hpp"
#include "../typeChecker/typeMaps.hpp"

namespace lsp
{
  inline std::vector<CompletionItem> globalCompletions = {
      {"int!", "Unsigned 64-bit integer.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"int?", "Signed 64-bit integer..", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"short!", "Unsigned 16-bit integer.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"short?", "Signed 16-bit integer..", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"char?", "Signed 8-bit integer.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"char!", "Unsigned 8-bit integer. Used for representing characters of a string..", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"str", "String type. A static list of characters.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"bool", "Boolean type. True or false.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"void", "No value type, used for functions that do not return a value.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"float", "32-bit floating-point number type.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"double", "64-bit floating-point number type.", "Type", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      // Builtin Functions with @ signs
      {"@cast", "Performs static casting to another type.", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@template", "Declares the following declaration to be templated.", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@import", "Uses the global declarations of the file at the given path.", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@link", "Tell the compiler to statically link a library with -l[NAME]", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@extern", "Declares an imported function by @link as visible", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@call", "Call an imported function from @link", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@output", "Write a chunk of data, in string form, to a given file", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@outputln", "Equivalent to @output, but a newline character is appended", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      // {"@read", "Input data from a socket", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      // {"@write", "", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@input", "Recieve ", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@free", "Frees a piece of heap memory", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@alloc", "Allocates a bit of heap memory", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@memcpy", "Copies the given number of bytes from one source to a destination", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@sizeof", "Gets the static size of a type or expression in bytes", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@getArgv", "Returns the argv, as a *[]str type", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@getArgc", "Returns the number of args, as a int! type", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@streq", "Compares for if two null-terminated strings are equal", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      // file management
      {"@open", "Uses the given path to open a file descriptor", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@close", "Closes the given file descriptor to be freed", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@socket", "Creates a file descriptor for creating socket connections", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@bind", "Declares the given socket file descriptor as connectable", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@listen", "Declares the given socket file descriptor as listenable", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@accept", "Returns the FD of new connections, if none found then waits", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@recv", "Inputs bytes from an accepted socket FD", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      {"@send", "Outputs bytes to the accepted socket FD", "Native Function", "", CompletionItemKind::Function, InsertTextFormat::PlainText},
      // Keywords
      {"and", "Logical AND operator.", "Snippet", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"nil", "Represents a pointer with the value of 0.", "Snippet", "", CompletionItemKind::Keyword, InsertTextFormat::PlainText},
      {"return", "Returns a value.", "Snippet", "return $0;", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"fn", "Defines a function.", "Snippet", "const $1 := fn($2) $3 {\n\t$0\n};", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"struct", "Defines a structured type.", "Snippet", "const $1 := struct {\n\t$0\n};", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"enum", "Defines an enumeration type.", "Snippet", "const $1 := enum {\n\t$0\n};", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"if", "Conditional statement.", "Snippet", "if ($1) {\n\t$0\n}", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"else", "Default case in an if-else chain.", "Snippet", "else {\n\t$0\n}", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"while", "Loop over a condition condition.", "Snippet", "loop ($1) {\n\t$0\n}", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
      {"for", "Loop over a condition condition with a post-loop operation..", "Snippet", "loop ($1): ($1++) {\n\t$0\n}", CompletionItemKind::Keyword, InsertTextFormat::Snippet},
  };
}

void lsp::handleMethodTextDocumentCompletion(const nlohmann::json &object)
{
  const auto &params = object["params"];
  URI uri = params["textDocument"]["uri"];
  if (!documents.contains(uri))
    return;
  std::string activationCharacter = params.value("context", nlohmann::json::object()).value("triggerCharacter", "");

  nlohmann::json completions = nlohmann::json::array();

  for (const auto &item : globalCompletions)
  {
    // If activationCharacter is "@", only show items starting with '@'
    if (activationCharacter == "@")
    {
      if (item.label[0] != '@')
        continue;
    }
    // If activationCharacter is ".", only show items NOT starting with '@'
    else if (activationCharacter == ".")
    {
      if (item.label[0] == '@')
        continue;
    }
    // Otherwise, if the character under the cursor matches the first char of label, show only those
    else if (activationCharacter.empty() || params["context"]["triggerKind"] == 1)
    {
      size_t line = params["position"]["line"];
      size_t character = params["position"]["character"];
      if (getCharUnder(documents[uri], line, character - 1) != item.label[0])
        continue;
      // Yay! We can move on!
      activationCharacter = item.label[0]; // Set the activation character to the first character of the label
    }
    std::string insertText = item.label;
    if (!item.snippet.empty()) insertText = item.snippet;
    else {
      if (insertText.starts_with("@") && activationCharacter == "@")
        insertText = insertText.substr(1); 
    }
    std::string sortText = item.label;
    if (item.kind == CompletionItemKind::Function) sortText = "3_" + sortText;
    else if (item.kind == CompletionItemKind::Keyword) sortText = "5_" + sortText;
    else if (item.kind == CompletionItemKind::Variable) sortText = "1_" + sortText;
    // It was something else, juts show everything.
    nlohmann::json entry = {
        {"label", item.label},
        {"kind", item.kind}, // Variable-like, customize if needed
        {"documentation", item.documentation},
        {"insertText", insertText},
        {"sortText", sortText}
    };

    if (!item.detail.empty())
      entry["detail"] = item.detail;

    if (!item.snippet.empty())
      entry["insertTextFormat"] = static_cast<int>(item.format); // 2 = Snippet

    completions.push_back(entry);
  }

  // If you hit the . key on your keyboard, that means you want to see all completionks
  // under whatever object you just typed. So, let's typecheck everything before the dot
  // until we reach a chracter that is not an ident or another dot
  size_t line = params["position"]["line"];
  size_t character = params["position"]["character"];
  if (activationCharacter == "." || (getCharUnder(documents[uri], line, character - 1) == '.'))
  {
    std::string text = documents[uri];
    signed long start = character;

    std::string identifier = "";
    while ((start - 2) >= 0) {
      char c = getCharUnder(text, line, (start - 2));
      start--;
      if (!isTokenCharacter(c)) {
        break;
      }
      identifier = c + identifier;
    }

    if (!identifier.empty())
    {
      // Hopefully, there is an LSP_ident with the type of the identifier you just typed
      TypeChecker::LSPIdentifier closestLSPIdent;
      closestLSPIdent.line = 18000000000000000;
      for (const auto &ident : TypeChecker::lsp_idents)
      {
        if (ident.ident == identifier && ident.fileID == fileIDFromURI(uri))
        {
          // Thats good! We just have to go deeper now...
          if (closestLSPIdent.line == 18000000000000000) {
            closestLSPIdent = ident;
            continue;
          }

          // We have to check if the line of the current ident is closer to the line of the dot
          // than the previous ident
          if (line - ident.line < line - closestLSPIdent.line ||
              (line - ident.line == line - closestLSPIdent.line && (character - ident.pos) < (character - closestLSPIdent.pos))) {
            closestLSPIdent = ident;
          }
        }
      }
      // The lsp ident contains the type of the identifier. Let's see now!
      if (closestLSPIdent.line != 18000000000000000) {
        // check the type of the underlying
        std::string typeStr = (closestLSPIdent.underlying->kind == ND_SYMBOL_TYPE ? 
                              TypeChecker::type_to_string(closestLSPIdent.underlying) :
                              closestLSPIdent.underlying->kind == ND_POINTER_TYPE ?
                              TypeChecker::type_to_string(static_cast<PointerType *>(closestLSPIdent.underlying)->underlying) : "");
        if (context->structTable.contains(typeStr)) {
          for (const auto &member : context->structTable.at(typeStr))
          {
            nlohmann::json entry = {
                {"label", member.first},
                {"kind", static_cast<int>(CompletionItemKind::Field)},
                {"documentation", "Struct member"},
                {"insertText", member.first},
                {"sortText", "2_" + member.first}
            };
            completions.push_back(entry);
          }
        }
        if (context->enumTable.contains(typeStr) || typeStr == "enum") {
          for (const auto &member : context->enumTable.at(typeStr=="enum"?closestLSPIdent.ident:typeStr))
          {
            nlohmann::json entry = {
                {"label", member.first},
                {"kind", static_cast<int>(CompletionItemKind::EnumMember)},
                {"documentation", "Enum member"},
                {"insertText", member.first},
                {"sortText", "2_" + std::to_string(member.second) + "_" + member.first}
            };
            completions.push_back(entry);
          }
        }
      }
    }
  }
  nlohmann::json response = {
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", {{"isIncomplete", false}, {"items", completions}}}};

  handleResponse(response);
}
