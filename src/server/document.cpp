#include <filesystem>
#include "lsp.hpp"
#include "../typeChecker/type.hpp"
#include "../typeChecker/typeMaps.hpp"
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
}

void lsp::clearDiagnostics() {
  // loop through each declared URI in the documents and forcefully tell the client that
  // there are no more diagnostics for that given URI
  for (const auto& [uri, _] : documents) {
    nlohmann::json clearResponse = {
      {"jsonrpc", "2.0"},
      {"method", "textDocument/publishDiagnostics"},
      {"params", {
        {"uri", uri},
        {"diagnostics", nlohmann::json::array()} // No diagnostics for cleared documents
      }}
    };
    handleResponse(clearResponse); // Send the response back to the client
  }
};

lsp::URI lsp::fix_broken_uri(lsp::URI uri) {
  // This function is used to fix broken URIs that start with "home/[xxx]/..." instead of "/home/[xxx]/..."
  if (uri.starts_with("home/")) {
    uri = "/" + uri; // Replace "home/" with "/home/"
  }
  // add more optimizations later
  return uri;
};

void lsp::reportErrors(std::vector<Error::ErrorInfo> errors, lsp::URI uri) {
  (void)uri;
  // 0 trust in copilot
  if (errors.empty()) {
    clearDiagnostics();
    return;
  }
  // Great! We have stuff to do....
  // a way of storing the errors for every given URI (file)
  std::map<lsp::URI, std::vector<Error::ErrorInfo>> uriErrors; // I feel so bad memory-wise, PLEASE forgive me!!!
  for (const Error::ErrorInfo &error : errors) {
    // issue where files will start with "home/[xxx]/..." instead of "/home/[xxx]/..."
    // so we need to make sure that the file_path is always a valid URI
    std::string fixedUri = fix_broken_uri(error.file_path);
    if (uriErrors.contains(fixedUri)) {
      uriErrors[fixedUri].push_back(error);
    } else {
      uriErrors[fixedUri] = {error};
    }
  }
  for (const auto& [fileUri, fileErrors] : uriErrors) {
    auto diagnostics = nlohmann::json::array();
    for (const auto& error : fileErrors) {
      diagnostics.push_back(nlohmann::json({
        {"range", {
          {"start", {
            {"line", error.line_start - 1}, // lines are 0-based
            {"character", error.col_start}  // so are columns
          }},
          {"end", {
            {"line", error.line_end - 1},
            {"character", error.col_end},
          }}
        }}, // </range>
        {"severity", error.simplified_message.find("warning") != std::string::npos ? DiagnosticSeverity::Warning : DiagnosticSeverity::Error},
        {"code", error.simplified_message.find("warning") != std::string::npos ? "Warning" : "Error"},
        {"source", "Zura builtin LSP"},
        {"message", error.simplified_message},
      }));
    }
    nlohmann::json errorResponse = {
      {"jsonrpc", "2.0"},
      {"method", "textDocument/publishDiagnostics"},
      {"params", {
        {"uri", fileUri},
        {"diagnostics", diagnostics}
      }}
    };
    handleResponse(errorResponse); // Send the response back to the client
    diagnostics.clear();
  }
  // clear the errors so far
  uriErrors.clear();
  return;
}

void lsp::handleMethodTextDocumentDidClose(const nlohmann::json& params) {
  // This method is called when a text document is closed
  URI uri = params["textDocument"]["uri"];
  
  documents.erase(uri); // Remove the document from the map
  logFile << "Closed document: " << uri << "\n" << std::flush;
  // tell the client that there are no more diagnostics (its just nice, but not required)
  nlohmann::json closeResponse = {
    {"jsonrpc", "2.0"},
    {"method", "textDocument/publishDiagnostics"},
    {"params", {
      {"uri", uri},
      {"diagnostics", nlohmann::json::array()} // No diagnostics for closed documents
    }}
  };
  handleResponse(closeResponse); // Send the response back to the client
}

void lsp::handleMethodTextDocumentDidSave(const nlohmann::json& params) {
  // This method is called when a text document is saved
  URI uri = params["textDocument"]["uri"];
  
  // We can log the content of the document if needed
  logFile << "Saved document: " << uri << "\n" << std::flush;

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
  std::string uriToCompile = uri;
  if (mainFileLink.contains(uri))  {
    uriToCompile = mainFileLink[uri];
  }
  logFile << "Coming from file " << uri << ", compiling " << uriToCompile << "\n";
  for (const auto& file : TypeChecker::importedFiles) {
    logFile << "Imported file: " << file << "\n";
  }
  for (const auto& [key, value] : mainFileLink) {
    logFile << "Main file link: " << key << " -> " << value << "\n";
  }
  
  std::string content = documents[uriToCompile];
  std::string reporterUri = uriToCompile.starts_with("file://") ? uriToCompile.substr(7) : uriToCompile;
  node.current_file = reporterUri;
  Node::Stmt *result = Parser::parse(content.c_str(), reporterUri);
  bool parserError = !Error::errors.empty();
  // Report those
  if (parserError) {
    reportErrors(Error::errors, uriToCompile);
    Error::errors.clear();
    return; 
  }

  TypeChecker::performCheck(result, uriToCompile == uri, true);

  // Add the current file to the main file link IF it has a main function
  if (uri == uriToCompile && TypeChecker::foundMain && !mainFileLink.contains(reporterUri)) {
    mainFileLink[uriToCompile] = uri; // Link the main file to itself
  }
  // check all imported files
  for (const auto& importedFile : TypeChecker::importedFiles) {
    if (!mainFileLink.contains(importedFile)) {
      mainFileLink["file://" + importedFile] = uriToCompile;
    }
  }
  bool tcError = !Error::errors.empty();
  // Only report type checker errors if there were no parser errors
  if (tcError) {
    reportErrors(Error::errors, uriToCompile);
    Error::errors.clear();
    return;
  } else {
    clearDiagnostics(); // globally clear diagnostics since there were none, globally
  }
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