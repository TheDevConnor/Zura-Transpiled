#include <filesystem>
#include "lsp.hpp"
#include "../codegen/gen.hpp"
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

nlohmann::json lsp::reportErrors(std::vector<Error::ErrorInfo> errors, lsp::URI uri, bool sendResponse) {
  (void)uri;
  // 0 trust in copilot
  if (errors.empty()) {
    clearDiagnostics();
    return nlohmann::json::array();
  }
  // Great! We have stuff to do....
  // a way of storing the errors for every given URI (file)
  std::map<lsp::URI, std::vector<Error::ErrorInfo>> uriErrors; // I feel so bad memory-wise, PLEASE forgive me!!!
  for (const Error::ErrorInfo &error : errors) {
    // issue where files will start with "home/[xxx]/..." instead of "/home/[xxx]/..."
    // so we need to make sure that the file_path is always a valid URI
    std::string fixedUri = fix_broken_uri(error.file_path);
    if (!fixedUri.starts_with("file://")) {
      fixedUri = "file://" + fixedUri;
    }

    if (uriErrors.contains(fixedUri)) {
      uriErrors[fixedUri].push_back(error);
    } else {
      uriErrors[fixedUri] = {error};
    }
  }
  auto diagnostics = nlohmann::json::array();
  for (const auto& [fileUri, fileErrors] : uriErrors) {
    for (const auto& error : fileErrors) {
      nlohmann::json diagnostic = {
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
      };
      diagnostics.push_back(diagnostic);
    }
    if (sendResponse) {
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
  }
  // clear the errors so far
  uriErrors.clear();
  return diagnostics;
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

  execDiagnostic(uri, true);
}

std::vector<Error::ErrorInfo> lsp::execDiagnostic(lsp::URI uri, bool sendResponse) {
  std::string uriToCompile = uri;
  if (mainFileLink.contains(uri))  {
    uriToCompile = mainFileLink[uri];
  }
  
  std::string content = documents[uriToCompile];
  std::string reporterUri = uriToCompile.starts_with("file://") ? uriToCompile.substr(7) : uriToCompile;
  node.current_file = reporterUri;
  Node::Stmt *result = Parser::parse(content.c_str(), reporterUri);
  bool parserError = !Error::errors.empty();
  // Report those
  if (parserError) {
    if (sendResponse)
      reportErrors(Error::errors, uriToCompile, true);
    // Return errors that are in the URI
    std::vector<Error::ErrorInfo> errors = Error::errors;
    Error::errors.clear();
    return errors;
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
    if (sendResponse)
      reportErrors(Error::errors, uriToCompile, true);
    std::vector<Error::ErrorInfo> typeErrors = Error::errors;
    Error::errors.clear();
    return typeErrors;
  } else {
    clearDiagnostics(); // globally clear diagnostics since there were none, globally
    return {}; // Empty
  }
}

void lsp::handleMethodTextDocumentCodeAction(const nlohmann::json& object) {
  auto codeActions = nlohmann::json::array();
  
  // Run checks on the file
  URI uri = object["params"]["textDocument"]["uri"];
  if (!documents.contains(uri)) {
    logFile << "⚠️ No document found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", codeActions} // Intentionally empty
    };
    handleResponse(response);
    return;
  }

  std::vector<Error::ErrorInfo> errors = execDiagnostic(uri, false);
  if (errors.empty()) {
    logFile << "No errors found in document: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", codeActions} // Intentionally empty
    };
    handleResponse(response);
    return;
  }
  // Uh oh, there were errors.
  for (const Error::ErrorInfo &error : errors) {
    nlohmann::json errorJSON = reportErrors({error}, uri, false); // Get the diagnostic version
    if (error.simplified_message.find("Did you mean") != std::string::npos) {
      std::string newText = error.simplified_message.substr(0, error.simplified_message.find_last_of('\''));
      newText = newText.substr(newText.find_last_of('\'') + 1);
      std::string textWithNewTextRemoved = error.simplified_message.substr(0, error.simplified_message.find("Did you mean"));

      std::string originalText = textWithNewTextRemoved.substr(0, textWithNewTextRemoved.find_last_of('\''));
      originalText = originalText.substr(originalText.find_last_of('\'') + 1);
      

      nlohmann::json codeAction = {
        {"title", "Change '" + originalText + "' to '" + newText + "'"},
        {"kind", "quickfix"},
        {"diagnostics", {errorJSON[0]}},
        {"edit", {
          {"changes", {
            {uri, {
              {{"range", errorJSON[0]["range"]}, {"newText", newText}} // This is a placeholder, you would replace it with the actual fix
            }}
          }}
        }}
      };
      codeActions.push_back(codeAction);
    }
  }

  auto response = nlohmann::json{
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", codeActions}
  };
  handleResponse(response);
  return;
}

void lsp::handleMethodTextDocumentSemanticTokensFull(const nlohmann::json& object) {
  // STEP 1 PRECAUTION: Make sure we have LSP identifiers in the first place
  logFile << "Semantic tokens called" << std::endl;
  if (TypeChecker::lsp_idents.empty()) {
    logFile << "⚠️ No LSP identifiers found, cannot provide semantic tokens.\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", {}} // Intentionally empty
    };
    handleResponse(response);
    return;
  }
  logFile << "Found idents" << std::endl;
  // This method is called to provide semantic tokens for the entire document
  URI uri = object["params"]["textDocument"]["uri"];
  if (!documents.contains(uri)) {
    logFile << "⚠️ No document found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", {}} // Intentionally empty
    };
    handleResponse(response);
    return;
  }
  logFile << "Found URI" << std::endl;
  
  // check what file the uri matches to
  size_t fileID = fileIDFromURI(uri);
  logFile << "ran the file id from uri function" << std::endl;
  // NOTE: this is the point! I guaruntee you don't have 18 quintillion imports.
  if (fileID == (size_t)-1) {
    logFile << "⚠️ No matching file ID found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", {}} // Intentionally empty
    };
    handleResponse(response);
    if (mainFileLink.contains(uri)) return;
    // Create a diagnostic for "Please open the main file that imports this one"
    nlohmann::json diagnostic = {
      {"range", {
        {"start", {{"line", 0}, {"character", 0}}},
        {"end", {{"line", 0}, {"character", 1}}}
      }},
      {"severity", DiagnosticSeverity::Information},
      {"code", "Error"},
      {"source", "Zura LSP"},
      {"message", "Open the source file that contains the main function to enable various features (like disabling this error!)"}
    };
    nlohmann::json errorResponse = {
      {"jsonrpc", "2.0"},
      {"method", "textDocument/publishDiagnostics"},
      {"params", {
        {"uri", uri},
        {"diagnostics", nlohmann::json::array({diagnostic})}
      }}
    };
    handleResponse(errorResponse); // Send the response back to the client
    return;
  }
  logFile << "Found file id" << std::endl;

  // Guess what? We have LSP_Identifiers!!! Those can work
  // The result is a bunch of uints
  // [ lineChange, characterChange, tokenLength, tokenType, tokenModifiers (always 0 here) ]
  auto tokens = nlohmann::json::array();
  std::vector<TypeChecker::LSPIdentifier> lspIdentsInURI = {};
  for (const auto& ident : TypeChecker::lsp_idents) {
    if (ident.fileID == fileID) {
      lspIdentsInURI.push_back(ident);
    }
  }
  size_t prevLine = 0;
  size_t prevCharacter = 0;
  // UGHH now we have to actually START
  for (const auto& ident : lspIdentsInURI) {
    if (ident.line == (size_t)-1) continue;
    if (ident.ident.empty()) continue;
    long lineChange = (ident.line - 1) - prevLine;
    long characterChange = ident.pos - prevCharacter;
    if (lineChange != 0) {
      characterChange = ident.pos;
    }
    if (lineChange < 0 || characterChange < 0) {
      logFile << "stupid token of name " << ident.ident << " on line  " << ident.line << " pos " << ident.pos << " fileID " << ident.fileID << " type " << (int)ident.type << std::endl;
    }
    size_t tokenLength = ident.ident.length();
    size_t tokenType = static_cast<size_t>(ident.type); // Convert enum to size_t
    size_t tokenModifiers = 0; // No modifiers for now
    // Add the token to the array
    tokens.push_back(lineChange);
    tokens.push_back(characterChange);
    tokens.push_back(tokenLength);
    tokens.push_back(tokenType);
    tokens.push_back(tokenModifiers);

    // Update previous line and character
    prevLine = ident.line - 1;
    prevCharacter = ident.pos;
  }
  logFile << "Done" << std::endl;
  TypeChecker::LSPIdentifier finalIdent = lspIdentsInURI.back();
  // Now we have the tokens, we can send them back
  nlohmann::json response = {
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", {
      {"data", tokens},
      // {"resultId", "0"} // Not really needed, but good practice
    }}
  };
  handleResponse(response);
  return;
}

void lsp::handleMethodTextDocumentReferences(const nlohmann::json& object) {
  // This method is called to find references to a symbol in the document
  URI uri = object["params"]["textDocument"]["uri"];
  size_t line = object["params"]["position"]["line"];
  size_t character = object["params"]["position"]["character"];
  
  if (!documents.contains(uri)) {
    logFile << "⚠️ No document found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", nlohmann::json::array()} // Intentionally empty
    };
    handleResponse(response);
    return;
  }

  size_t fileID = fileIDFromURI(uri);
  if (fileID == (size_t)-1) {
    logFile << "⚠️ No matching file ID found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", nlohmann::json::array()} // Intentionally empty
    };
    handleResponse(response);
    return;
  }

  Word word = getWordUnder(documents[uri], line, character);
  if (word.text.empty()) {
    logFile << "⚠️ No word found at position (" << line << ", " << character << ") in document: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", nlohmann::json::array()} // Intentionally empty
    };
    handleResponse(response);
    return;
  }
  // Check if, under the position you clicked, there is an LSP identifier there
  TypeChecker::LSPIdentifier found;
  found.line = (size_t)-1;
  found.scope = "\0";
  for (const auto& ident : TypeChecker::lsp_idents) {
    if (ident.ident == word.text && ident.fileID == fileID && ident.line - 1 == line
        && ident.pos == word.range.start.character) {
      found = ident;
      break;
    }
  }
  auto locations = nlohmann::json::array();
  for (const auto& ident : TypeChecker::lsp_idents) {
    if (ident.ident == word.text && ((ident.scope == found.scope || ident.scope == "" || found.scope == "") || found.line == (size_t)-1)) { // If there was no found token, then ignore
      nlohmann::json location = {
        // get the uri of the file id
        {"uri", "file://" + (std::filesystem::path(codegen::fileIDs[0]) / codegen::fileIDs.at(ident.fileID)).string()},
        {"range", {
          {"start", {{"line", ident.line - 1}, {"character", ident.pos}}},
          {"end", {{"line", ident.line - 1}, {"character", ident.pos + word.text.length()}}}
        }}
      };
      locations.push_back(location);
    }
  }

  handleResponse({
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", locations}
  });
  return;
}

void lsp::handleMethodTextDocumentDefinition(const nlohmann::json& object) {
  // CHeck if the document exists
  URI uri = object["params"]["textDocument"]["uri"];
  if (!documents.contains(uri)) {
    logFile << "⚠️ No document found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", nullptr} // Intentionally null
    };
    handleResponse(response);
    return;
  }

  size_t line = object["params"]["position"]["line"];
  size_t character = object["params"]["position"]["character"];
  size_t fileID = fileIDFromURI(uri);
  if (fileID == (size_t)-1) {
    logFile << "⚠️ No matching file ID found for URI: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", nullptr} // Intentionally null
    };
    handleResponse(response);
    return;
  }
  Word word = getWordUnder(documents[uri], line, character);
  if (word.text.empty()) {
    logFile << "⚠️ No word found at position (" << line << ", " << character << ") in document: " << uri << "\n";
    auto response = nlohmann::json{
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", nullptr} // Intentionally null
    };
    handleResponse(response);
    return;
  }
  // Check the token under the word;
  TypeChecker::LSPIdentifier found;
  found.line = (size_t)-1;
  found.scope = "\0";
  found.isDefinition = false;

  for (const auto& ident : TypeChecker::lsp_idents) {
    if (ident.ident == word.text && ident.fileID == fileID && ident.line - 1 == line
        && ident.pos == word.range.start.character) {
      found = ident;
      break;
    }
  }
  if (found.isDefinition) {
    // You already clicked on the defnitiion!
    nlohmann::json result = {
      {"uri", "file://" + (std::filesystem::path(codegen::fileIDs[0]) / codegen::fileIDs.at(found.fileID)).string()},
      {"range", {
        {"start", {{"line", word.range.start.line}, {"character", word.range.start.character}}},
        {"end", {{"line", word.range.end.line}, {"character", word.range.end.character}}}
      }}
    };
    handleResponse({
      {"jsonrpc", "2.0"},
      {"id", object["id"]},
      {"result", result}
    });
    return;
  }
  // check each token to see if the isDefinition is true
  for (const auto &ident : TypeChecker::lsp_idents) {
    if (ident.ident == word.text && ident.isDefinition && (ident.scope == "" || found.scope == "" || ident.scope == found.scope)) {
      // We found the definition, now we can return it
      nlohmann::json result = {
        {"uri", "file://" + (std::filesystem::path(codegen::fileIDs[0]) / codegen::fileIDs.at(ident.fileID)).string()},
        {"range", {
          {"start", {{"line", ident.line - 1}, {"character", ident.pos}}},
          {"end", {{"line", ident.line - 1}, {"character", ident.pos + word.text.length()}}}
        }}
      };
      handleResponse({
        {"jsonrpc", "2.0"},
        {"id", object["id"]},
        {"result", result}
      });
      return;
    }
  }
  // if we reach here then there was no definition it was bad
  logFile << "⚠️ No definition found for word: " << word.text << " at position (" << line << ", " << character << ") in document: " << uri << "\n";
  auto response = nlohmann::json{
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", nullptr} // Intentionally null
  };
  handleResponse(response);
  return;
};

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