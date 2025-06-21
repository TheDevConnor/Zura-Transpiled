#include <iostream>
#include <unordered_map>
#include "lsp.hpp"
#include "../common.hpp"
#include "../helper/flags.hpp"

using namespace nlohmann;
lsp::ServerStatus lspStatus = lsp::ServerStatus::Waiting; // Waiting to be initialized

void lsp::main() {
  shouldPrintErrors = false;
  shouldUseColor = false;
  logFile.open("/tmp/lsp.log", std::ios::out | std::ios::app);
  logFile << "-==-= Starting language server =-==-\n" << std::flush;
  //? Initialization!!!!
  //? We have to start listening to stdin
  std::string buffer = ""; //! There is no guarantee that the message will contained in full
  char line;
  size_t expectedLength = 0;
  while (lspStatus != ServerStatus::Exiting) {
    // Infinite loop! The only thing that can stop us a request to exit or an interrupt
    std::cin.get(line);
    if (line == '\0') continue;
    buffer += line;
    // If the Content-Length header is present, we must read the number from it]
    if (buffer.find("\r\n\r\n") == std::string::npos) continue;
    std::string firstLine = buffer.substr(0, buffer.find("\r\n"));
    expectedLength = std::stoi(firstLine.substr(16, firstLine.size()));
    logFile << line << std::flush;
    if (buffer.size() < expectedLength + firstLine.size() + 4) continue; // Add 4 because \r\n\r\n
    logFile << "\n" << std::flush;
    // Now that we have the message, we must parse it (it is just JSON, you know)
    try {
      auto parsed = json::parse(buffer.substr(firstLine.size() + 4, firstLine.size() + 4 + expectedLength));

      if (parsed.contains("method")) {
        std::string method = parsed["method"];
        // Handle the method accordingly
        handleMethod(method, parsed);
      }
    } catch (const nlohmann::json::parse_error& e) {
      logFile << "JSON parse error: " << e.what() << "\n" << std::flush;
    }
    buffer = buffer.substr(expectedLength + firstLine.size() + 4); // Remove the processed message from the buffer
  }
  logFile << "-==-= Exiting language server =-==-\n" << std::flush;
  logFile.close();
  return;
}

void lsp::handleMethod(const std::string& method, const nlohmann::json& params) {
  if (method == "initialize") {
    handleMethodInitialize(params);
  } else if (method == "shutdown") {
    handleMethodShutdown(params);
  } else if (method == "exit") {
    handleMethodExit(params);
  } else if (method == "textDocument/didOpen") {
    handleMethodTextDocumentDidOpen(params["params"]);
  } else if (method == "textDocument/didChange") {
    handleMethodTextDocumentDidChange(params["params"]);
  } else if (method == "textDocument/didClose") {
    handleMethodTextDocumentDidClose(params["params"]);
  } else if (method == "textDocument/didSave") {
    handleMethodTextDocumentDidSave(params["params"]);
  } else if (method == "textDocument/hover") {
    handleMethodTextDocumentHover(params);
  } else if (method == "textDocument/diagnostic") {
    nlohmann::json response = {
      {"jsonrpc", "2.0"},
      {"id", params["id"]},
      {"result", reportErrors(execDiagnostic(params["params"]["textDocument"]["uri"], false), params["params"]["textDocument"]["uri"], false) } // do NOT send as a notification 
    };
    handleResponse(response);
  } else if (method == "textDocument/completion") {
    handleMethodTextDocumentCompletion(params);
  } else if (method == "textDocument/codeAction") {
    handleMethodTextDocumentCodeAction(params);
  } else if (method == "textDocument/semanticTokens/full") {
    handleMethodTextDocumentSemanticTokensFull(params);
  } else if (method == "textDocument/semanticTokens/range") {
    // Here we would typically return semantic tokens for a specific range in the document
    // For now, we just send an empty response
    nlohmann::json response = {
      {"jsonrpc", "2.0"},
      {"id", params["id"]},
      {"result", {
        {"data", {}}, // No tokens for now
        {"resultId", "0"} // Not really needed, but good practice
      }}
    };
    handleResponse(response);
  } else if (method == "textDocument/references") {
    handleMethodTextDocumentReferences(params);
  } else if (method == "textDocument/definition") {
    handleMethodTextDocumentDefinition(params);
  } else {
    logFile << "Unknown method: " << method << "\n" << std::flush;
    // We can send back an error response, but for now we just ignore it
  }
}

void lsp::handleResponse(const nlohmann::json& response) {
  // Here we would typically send the response back to the client
  // For now, we just print it to stdout
  std::string responseStr = response.dump();
  std::cout << "Content-Length: " << std::to_string(responseStr.size()) << "\r\n\r\n" // The LSP protocol requires a Content-Length header
            << responseStr // The LSP protocol requires a double newline at the end
            << std::flush;
  logFile << "-==-= Server Response =-==-\n\n" << responseStr << "\n" << std::flush;
  logFile << "-==-= Client Request =-==-\n" << std::flush;
}

void lsp::handleMethodInitialize(const nlohmann::json &object) {
  // Here we would typically set up the server, capabilities, etc.
  json response = {
      {"jsonrpc", "2.0"},   // Not required, but good practice
      {"id", object["id"]}, // Required to send back
      {"result",
       {{"capabilities",
         {                                // Be as USELESS as possible!!
          {"positionEncoding", "utf-16"}, // To make VSCode happy
          {"hoverProvider", true},
          {"documentLinkProvider", false},
          {"definitionProvider", true},
          {"referencesProvider", true},
          {"documentSymbolProvider", false},
          {"workspaceSymbolProvider", false},
          {"textDocumentSync", TextDocumentSyncKind::Incremental},
          {"completionProvider", {
            {"resolveProvider", true},
            {"triggerCharacters", {".", "@"}} // Optional
          }},
          {"declarationProvider", false},
          {"typeDefinitionProvider", false},
          {"implementationProvider", false},
          {"renameProvider", false},
          {"codeActionProvider", {
          {"codeActionKinds", {"quickfix"}},
          {"resolveProvider", true}
          }},
          {"documentFormattingProvider", false},
          {"documentRangeFormattingProvider", false},
          {"semanticTokensProvider", {
          {"legend", {
            {"tokenTypes", {
              "function",       // Function
              "struct",         // Struct
              "enum",           // Enum
              "variable",       // Variable
              "enumMember",     // EnumMember
              "property",       // StructMember (property is the official name)
              "method",         // StructFunction (method is the official name)
              "type",           // Type is generic so it will be our "unknown"
            }},
            // {"tokenModifiers", {"readonly", "static"}}
            {"tokenModifiers", {}},
          }},
          {"range", false}, // We dont support range tokens
          {"full", true}   // We support full semantic tokens
        }},
        {"documentLinkProvider", {{"resolveProvider", false}}},
        {"colorProvider", false},
        {"foldingRangeProvider", false},
        {"diagnosticProvider", {
          {"relatedInformation", false},
          {"tagSupport",
            {
                {"valueSet", {1, 2}} // not really needed here
            }},
          {"versionSupport", false},
          {"codeDescriptionSupport", false},
          {"dataSupport", false}
        }
      }}},
      {"serverInfo",
        {{"name", "Zura LSP"},
        {"version", "0.0.1"}}
      }}
    }
  };
  handleResponse(response);
  lspStatus = ServerStatus::Initialized;
}

void lsp::handleMethodShutdown(const nlohmann::json& object) {
  // Here we would typically clean up resources, etc.
  json response = {
    {"jsonrpc", "2.0"},
    {"id", object["id"]},
    {"result", nullptr} // Shutdown doesnt have any required response structure, so you usually send back "null"
  };
  handleResponse(response);
  lspStatus = ServerStatus::Stopped; // We are stopping the server
}

void lsp::handleMethodExit(const nlohmann::json& object) {
  // There's actually no need to send anything back, as if an exit
  // method was sent that means that the client wouldn't be
  // listening to us anymore anyways
  (void)object;
  lspStatus = ServerStatus::Exiting; // We are exiting the server
}