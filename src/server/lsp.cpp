#include <iostream>
#include <unordered_map>
#include "lsp.hpp"
#include "logging.hpp"

void lsp::initialize() {
  lsp::main();
};

using Method = std::function<nlohmann::ordered_json(nlohmann::json&)>;
using Event = std::function<void(nlohmann::json&)>;
const static std::unordered_map<std::string, Method> methodMap = {
  {"initialize", lsp::methods::initialize},
  {"shutdown", lsp::methods::shutdown},
  {"exit", lsp::methods::exit},
  {"textDocument/hover", lsp::methods::hover},
  {"textDocument/completion", lsp::methods::completion},
  {"$/cancelRequest", lsp::methods::cancelRequest}
};
const static std::unordered_map<std::string, Event> eventMap = {
  {"textDocument/didOpen", lsp::events::documentOpen},
  {"textDocument/didChange", lsp::events::documentChange}
};

void lsp::main() {
  std::string buffer = "";
  char ch;
  logging::init();
  while (std::cin.get(ch)) {
    // See if there is any new input from stdin
    buffer += ch;
    // See if we have "Content-Length" header included
    logging::log(ch);
    if (buffer.find("\r\n\r\n") == std::string::npos) {
      continue;
    }
    // Content-Length header is fully in there

    std::string firstLineOfBuffer = buffer.substr(0, buffer.find("\r\n\r\n"));
    int contentLength = std::stoi(firstLineOfBuffer.substr(16, firstLineOfBuffer.size()));
    if (buffer.size() < contentLength + firstLineOfBuffer.size() + 4) {
      continue;
    }

    // JSON parse the message
    std::string message = buffer.substr(buffer.find("\r\n\r\n") + 4, buffer.find("\r\n\r\n") + 4 + contentLength);
    logging::log("\n"); // extra newline in logfile after Client request for readability
    // Log the message
    nlohmann::json request = nlohmann::json::parse(message);
    // Only use std out for replying to the client
    if (request.contains("method")) {
      logging::log("Method: " + std::string(request["method"]) + "\n");
      if (methodMap.contains(request.at("method"))) {
        nlohmann::ordered_json response = methodMap.at(request.at("method"))(request);
        nlohmann::ordered_json finalResult = {
          {"jsonrpc", "2.0"},
          {"result", response}
        };
        if (request.contains("id")) {
          finalResult["id"] = request["id"];
        }
        std::string responseStr = finalResult.dump();
        std::string header = "Content-Length: " + std::to_string(responseStr.size()) + "\r\n\r\n";
        logging::log (header + responseStr + "\n");
        std::cout << (header + responseStr) << std::flush;
      } else if (eventMap.contains(request.at("method"))) {
        // These do not return results and are OK to invoke just like this
        eventMap.at(request.at("method"))(request);
      }
    }

    buffer = buffer.substr(buffer.find("\r\n\r\n") + 4 + contentLength);
  };
  logging::close();
};