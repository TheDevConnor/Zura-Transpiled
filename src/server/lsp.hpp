#pragma once
#include <fstream>
#include <unordered_map>
#include <map>
#include "json.hpp"
#include "../helper/error/error.hpp"

namespace lsp {
  using URI = std::string;
  inline std::ofstream logFile;
  inline bool active = false;
  inline std::unordered_map<URI, URI> mainFileLink {}; // links the URI of a module to the main file that imports it (i.e the file with the main function) 
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
  extern std::map<URI, std::string> documents;
  size_t fileIDFromURI(URI uri);
  size_t getOffset(const std::string& text, size_t line, size_t character); // Calculates the offset in the text based on line and character
  nlohmann::json reportErrors(std::vector<Error::ErrorInfo> errors, URI uri, bool sendResponse=false);
  std::vector<Error::ErrorInfo> execDiagnostic(URI uri, bool sendResponse=false); // Executes diagnostics on the document at the given URI
  void clearDiagnostics(); // Clears all diagnostics for all documents
  URI fix_broken_uri(URI uri); // Fixes broken URIs that start with "home/[xxx]/..." instead of "/home/[xxx]/..."
  void handleMethodTextDocumentDidOpen(const nlohmann::json& params); // Handles the "textDocument/didOpen" method
  void handleMethodTextDocumentDidChange(const nlohmann::json& params); // Handles the "textDocument/didChange" method
  void handleMethodTextDocumentDidClose(const nlohmann::json& params); // Handles the "textDocument/didClose" method
  void handleMethodTextDocumentDidSave(const nlohmann::json& params); // Handles the "textDocument/didSave" method
  void handleMethodTextDocumentCodeAction(const nlohmann::json& object);
  void handleMethodTextDocumentSemanticTokensFull(const nlohmann::json& object);
  //- METHODS: HOVERS & COMPLETIONS
  std::string getBuiltinNote(const std::string& builtin); // Returns a note about the builtin function
  bool isTokenCharacter(char c); // Checks if the character is a valid token character (alphanumeric or underscore)
  void handleMethodTextDocumentHover(const nlohmann::json& object); // Handles the "textDocument/hover" method
  void handleMethodTextDocumentCompletion(const nlohmann::json& object); // Handles the "textDocument/completion" method

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

  enum class InsertTextFormat {
    PlainText = 1,
    Snippet = 2
  };

  enum class CompletionItemKind {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25,
  };

  struct CompletionItem {
    std::string label;
    std::string documentation;
    std::string detail;
    std::string snippet;
    CompletionItemKind kind;
    InsertTextFormat format;
  };

  struct TextEdit {
    Range range;
    std::string newText; // what to replace what with
  };

  struct WorkspaceEdit {
    // changes: { [uri: DocumentUri]: TextEdit[]; }; // This means a typical 'changes' object looks like {"changes":{"file://...", change}}
    nlohmann::json changes; // Maps a URI to a JSON array of text edits
    // documentChanges: will be unused for now
    // changeAnnotations: will be unused for now
  };

  struct CodeAction {
    std::string title;
    std::string kind = "quickfix"; // Default kind is 'quickfix'; honestly, refactoring is YOUR problem, not ours
    nlohmann::json diagnostics; // Diagnostics that this code action addresses
    bool isPreferred = true; // There will be only a few code actions, so we can say they are all "preferred"
    // disabled: { std::string reason; } // If the code action was disabled, we wouldnt be displaying it obviously
    WorkspaceEdit edit; // The edit to apply to the document
  };


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

  Word getWordUnder(const std::string& text, size_t line, size_t character);
  char getCharUnder(const std::string& text, size_t line, size_t character);
}