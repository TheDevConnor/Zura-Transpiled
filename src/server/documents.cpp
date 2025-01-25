#include "./lsp.hpp"
#include <unordered_map>

// URI : Contents
std::unordered_map<std::string, std::string> documents = {};

std::vector<std::string> lsp::split(std::string in, std::string delim) {
  // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  while ((pos = in.find(delim)) != std::string::npos) {
      token = in.substr(0, pos);
      tokens.push_back(token);
      in.erase(0, pos + delim.length());
  }
  tokens.push_back(in);

  return tokens;
};

void lsp::document::setCharAtPos(std::string uri, Position pos, char changeChar) {
  int lineNum = pos.line;
  int column = pos.character;

  if (!documents.contains(uri)) return;
  std::string document = documents[uri];
  std::vector<std::string> lines = lsp::split(document, "\n");
  while (lines.size() <= lineNum) {
    lines.push_back("");
  };
  std::string line = lines.at(lineNum);
  while (line.size() <= column) {
    line += " ";
  };

  std::string newLine = line.substr(0, column) + changeChar + line.substr(column + 1);
  lines.at(lineNum) = newLine;

  std::string newDoc = "";
  for (int i = 0; i < lines.size(); i++) {
    newDoc += lines.at(i);
    if (i != lines.size() - 1) newDoc += "\n";
  };
  documents[uri] = newDoc;
}

char lsp::document::charAtPos(std::string uri, Position pos) {
  if (!documents.contains(uri)) return '\0';
  std::vector<std::string> lines = lsp::split(documents[uri], "\n");
  if (pos.line >= lines.size()) return '\0';
  std::string line = lines.at(pos.line);
  if (pos.character >= line.size()) return '\0';
  return line.at(pos.character);
};

void lsp::events::documentOpen(nlohmann::json& request) {
  std::string uri = request["params"]["textDocument"]["uri"];
  std::string contents = request["params"]["textDocument"]["text"];
  if (contents.empty()) {
    documents.emplace(uri, "\n"); // the newline is very important here
  } else {
    documents.emplace(uri, contents);
  }
};

void lsp::events::documentChange(nlohmann::json& request) {
  if (request["params"]["contentChanges"].size() == 1 && (!request["params"]["contentChanges"][0].contains("range"))) {
    // No range property means that Full TextDocumentChange is used - Just set directly
    std::string uri = request["params"]["textDocument"]["uri"];
    std::string contents = request["params"]["contentChanges"][0]["text"];
    if (contents.empty()) {
      documents[uri] = "\n"; // the newline is very important here
    } else {
      documents[uri] = contents;
    }
    return;
  }

  for (auto& change : request["params"]["contentChanges"]) {
    if (change["range"]["start"]["line"] == change["range"]["end"]["line"] && change["range"]["start"]["character"] == change["range"]["end"]["character"]) {
      // Only one character was changed in incremental mode
      std::string uri = request["params"]["textDocument"]["uri"];
      Position pos = {change["range"]["start"]["line"], change["range"]["start"]["character"]};
      char changeChar = std::string(change["text"]).at(0);
      lsp::document::setCharAtPos(uri, pos, changeChar);
      continue;
    }
    // Many characters were changed
    // In incremental mode
    std::vector<std::string> lines = lsp::split(change["text"], "\n");
    int start = change["range"]["start"]["line"];
    int end = change["range"]["end"]["line"];
    int startChar = change["range"]["start"]["character"];
    int endChar = change["range"]["end"]["character"];
    std::string uri = request["params"]["textDocument"]["uri"];
    std::string oldDoc = documents[uri];
    if (oldDoc.empty()) continue;
    std::vector<std::string> newDoc = lsp::split(oldDoc, "\n");
    if (start == end && startChar == endChar) {
      // Single line change
      lsp::document::setCharAtPos(uri, {start, startChar}, lines[0].at(0));
    } else {
      // A range and text was provided
      if (start == end) {
        newDoc[start] = newDoc[start].substr(0, startChar) + lines[0] + newDoc[start].substr(endChar);
      } else {
        if (end - start > 1) {
          // If the range is more than 1, we must delete the lines in between
          newDoc[start] = newDoc[start].substr(0, startChar) + lines[0];
          newDoc[end] = newDoc[end].substr(endChar);
          newDoc.erase(newDoc.begin() + start + 1, newDoc.begin() + end + 1);
        } else {
          // Remove the linebreak in between
          newDoc[start] = newDoc[start].substr(0, startChar) + lines[0];
          newDoc[end] = newDoc[end].substr(endChar);
          newDoc.erase(newDoc.begin() + end);
        }
      }
    }
    std::string result;
    for (int i = 0; i < newDoc.size(); i++) {
      result += newDoc[i];
      if (i != newDoc.size() - 1) result += "\n";
    };
    if (result.empty()) result = "\n";
    documents[uri] = result;
  }
}