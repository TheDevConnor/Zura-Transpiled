#pragma once
#include <fstream>
#include <map>
#include "json.hpp"
#include "../helper/error/error.hpp"

namespace lsp {
  inline std::ofstream logFile;
  inline bool active = false;
  void main(); // Initializes the LSP to start listening to stdout
  void handleMethod(const std::string& method, const nlohmann::json& object); // Handles the method received from the client
  void handleResponse(const nlohmann::json& response); // This is what we send back to the client. More often than not, its usually just null
  // NOTE: 'object' is for regular methods that want a response, while 'params' is for events, which are
  // NOTE: methods that do not expect a response.
  //- METHODS: LIFECYCLE
  void handleMethodInitialize(const nlohmann::json& object); // Handles the "initialize" method
  void handleMethodShutdown(const nlohmann::json& object); // Handles the "shutdown" method
  void handleMethodExit(const nlohmann::json& object); // Handles the "exit" method
  //- METHODS: DOCUMENTS
  using URI = std::string;
  extern std::map<URI, std::string> documents;
  size_t getOffset(const std::string& text, size_t line, size_t character); // Calculates the offset in the text based on line and character
  void reportErrors(std::vector<Error::ErrorInfo> errors, URI uri);
  void execDiagnostic(URI uri); // Executes diagnostics on the document at the given URI
  void handleMethodTextDocumentDidOpen(const nlohmann::json& params); // Handles the "textDocument/didOpen" method
  void handleMethodTextDocumentDidChange(const nlohmann::json& params); // Handles the "textDocument/didChange" method
  void handleMethodTextDocumentDidClose(const nlohmann::json& params); // Handles the "textDocument/didClose" method
  void handleMethodTextDocumentDidSave(const nlohmann::json& params); // Handles the "textDocument/didSave" method
  //- METHODS: HOVERS & COMPLETIONS
  std::string getBuiltinNote(const std::string& builtin); // Returns a note about the builtin function
  bool isTokenCharacter(char c); // Checks if the character is a valid token character (alphanumeric or underscore)
  void handleMethodTextDocumentHover(const nlohmann::json& object); // Handles the "textDocument/hover" method
  // void handleMethodTextDocumentCompletion(const nlohmann::json& object); // Handles the "textDocument/completion" method

  enum class ServerStatus {
    Unknown,
    
    Waiting,
    Initialized,
    Running,
    
    Stopped,
    Exiting,
  };

  enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
  };

  enum class DiagnosticTag {
    Unnecessary = 1,
    Deprecated = 2,
  };

  enum class TextDocumentSyncKind {
    None = 0,        // Dont send any events
    Full = 1,        // Send events, and contain the entirety of the new document
    Incremental = 2, // Send events, and only contain the parts of the document that changed
  };

  struct Document {
    std::string contents; // The text contents of the document
    // A date: This will be useful so we know when to check the document again
    std::time_t lastModified; // The last time the document was modified
  };

  struct Position {
    size_t line;
    size_t character;
  };

  struct Range {
    Position start; // The starting position of the range
    Position end;   // The ending position of the range
  };

  struct Word {
    std::string text; // The text of the word
    Range range;      // The range of the word in the document
  };

  Word getWordUnder(const std::string& text, size_t line, size_t character);
}