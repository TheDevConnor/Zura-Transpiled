#include "lsp.hpp"
#include "../typeChecker/type.hpp"
#include "../parser/parser.hpp"
#include "../helper/error/error.hpp"

std::map<lsp::URI, std::string> lsp::documents = {};

void lsp::handleMethodTextDocumentDidOpen(const nlohmann::json& params) {
  // This method is called when a text document is opened
  URI uri = params["textDocument"]["uri"];
  std::string content = params["textDocument"]["text"];
  
  documents.emplace(uri, content);
  logFile << "Opened document: " << uri << "\n" << std::flush;
}

void lsp::handleMethodTextDocumentDidChange(const nlohmann::json& params) {
  URI uri = params["textDocument"]["uri"];
  auto& document = documents[uri];

  if (params["contentChanges"].is_array()) {
    for (const auto& change : params["contentChanges"]) {
      std::string newText = change["text"];

      if (change.contains("range")) {
        // Incremental update
        const auto& range = change["range"];
        size_t startLine = range["start"]["line"];
        size_t startChar = range["start"]["character"];
        size_t endLine = range["end"]["line"];
        size_t endChar = range["end"]["character"];

        size_t startPos = getOffset(document, startLine, startChar);
        size_t endPos = getOffset(document, endLine, endChar);

        if (startPos != std::string::npos && endPos != std::string::npos && startPos <= endPos) {
          document.replace(startPos, endPos - startPos, newText);
        } else {
          logFile << "⚠️ Invalid range for document change at " << uri << "\n";
        }
      } else {
        // Full document replace
        document = newText;
      }
    }
  } else if (params["contentChanges"].contains("text")) {
    // Full document replace
    document = params["contentChanges"]["text"];
  }

  logFile << "Changed document: " << uri << "\n";
  logFile << "New content:\n" << document << "\n" << std::flush;
}

void lsp::reportErrors(std::vector<Error::ErrorInfo> errors, lsp::URI uri) {
  // 0 trust in copilot
  if (errors.empty()) {
    nlohmann::json errorResponse = {
      {"jsonrpc", "2.0"},
      {"method", "textDocument/publishDiagnostics"},
      {"params", {
        {"uri", ""}, // empty quotes means project-wide, I THINK**
        {"diagnostics", nlohmann::json::array()}
      }}
    };
  }
  // Great! We have stuff to do....
  auto diagnostics = nlohmann::json::array();
  for (const Error::ErrorInfo &error : errors) {
    diagnostics.push_back(nlohmann::json({
      {"range", {
        {"start", {
          {"line", error.line_start},
          {"character", error.col_start}
        }},
        {"end", {
          {"line", error.line_end},
          {"character", error.col_end},
        }}
      }}, // </range>
      {"severity", error.simplified_message.find("warning") != std::string::npos ? DiagnosticSeverity::Warning : DiagnosticSeverity::Error},
      {"code", error.simplified_message.find("warning") != std::string::npos ? "Warning" : "Error"},
      {"source", "Zura builtin LSP"},
      {"message", error.simplified_message},
      {"filePath", error.file_path}
    }));
  }
  nlohmann::json errorResponse = {
    {"jsonrpc", "2.0"},
    {"method", "textDocument/publishDiagnostics"},
    {"params", {
      {"uri", uri},
      {"diagnostics", diagnostics}
    }}
  };
  handleResponse(errorResponse); // Send the response back to the client
}

void lsp::handleMethodTextDocumentDidClose(const nlohmann::json& params) {
  // This method is called when a text document is closed
  URI uri = params["textDocument"]["uri"];
  
  documents.erase(uri); // Remove the document from the map
  logFile << "Closed document: " << uri << "\n" << std::flush;
}

void lsp::handleMethodTextDocumentDidSave(const nlohmann::json& params) {
  // This method is called when a text document is saved
  URI uri = params["textDocument"]["uri"];
  
  // We can log the content of the document if needed
  logFile << "Saved document: " << uri << "\n" << std::flush;
  logFile << "Content: " << documents[uri] << "\n" << std::flush;

  // TypeCheck the entire file
  // we also know that the file exists because in order to save you would have to have it open
  std::string content = documents[uri];
  if (content.empty()) {
    logFile << "⚠️ Document is empty: " << uri << "\n";
    return;
  }
  logFile << "Type checking document: " << uri << "\n";
  // run a diagnostic

  execDiagnostic(uri);
}

void lsp::execDiagnostic(lsp::URI uri) {
  std::string content = documents[uri];
  Node::Stmt *result = Parser::parse(content.c_str(), uri.substr(7));
  bool parserError = Error::errors.size() > 0;
  // Report those
  if (parserError) return reportErrors(Error::errors, uri);

  TypeChecker::performCheck(result);
  bool tcError = Error::errors.size() > 0;
  // Report those
  if (tcError) return reportErrors(Error::errors, uri);
  reportErrors({}, uri); // We are clean!!
}

size_t lsp::getOffset(const std::string& text, size_t line, size_t character) {
  size_t offset = 0;
  std::istringstream stream(text);
  std::string lineStr;
  for (size_t i = 0; i < line; ++i) {
    if (!std::getline(stream, lineStr)) return std::string::npos;
    offset += lineStr.length() + 1; // +1 for the newline
  }
  offset += character;
  return offset;
}