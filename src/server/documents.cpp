#include <unordered_map>
#include "../parser/parser.hpp"
#include "./lsp.hpp"

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

lsp::Word lsp::document::wordUnderPos(std::string uri, Position pos) {
  // neither of these will basically ever be negative
  size_t lineNum = pos.line;
  size_t column = pos.character;

  if (!documents.contains(uri)) return {};
  std::string document = documents[uri];
  std::vector<std::string> lines = lsp::split(document, "\n");
  if (lineNum >= lines.size()) return {};
  std::string line = lines.at(lineNum);
  if (column >= line.size()) return {};
  // get all characters surrounding the current one until a whitespace or special character is found
  // _ and - are allowed
  std::string word = "";
  size_t firstCol = 0;
  for (size_t i = column; i > 0; i--) {
    char c = line.at(i);
    firstCol = i;
    if (c == '\n') break;
    if ((!std::isalnum(c)) && c != '_' && c != '-' && c != '@') {
      break;
    }
    word = c + word;
  };
  size_t lastCol = line.size();
  for (size_t i = column + 1; i < line.size(); i++) {
    char c = line.at(i);
    lastCol = i;
    if (c == '\n') break;
    if ((!std::isalnum(c)) && c != '_' && c != '-' && c != '@') {
      break;
    }
    word += c;
  };

  return Word { .word = word, .range = { .start = { .line = lineNum, .character = firstCol }, .end = { .line = lineNum, .character = lastCol } } };
};

void lsp::document::setCharAtPos(std::string uri, Position pos, std::string changeChar) {
  size_t lineNum = pos.line;
  size_t column = pos.character;

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
  for (size_t i = 0; i < lines.size(); i++) {
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
  TypeChecker::performCheck(Parser::parse(contents.c_str(), uri), false, true);
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
    // check if teh character was a newline or a semicolon (typecheck again if so)
    if (contents.empty()) {
      documents[uri] = "\n"; // the newline is very important here
    } else {
      documents[uri] = contents;
    }
    TypeChecker::performCheck(Parser::parse(lsp::document::getText(request["params"]["textDocument"]["uri"]).c_str(), request["params"]["textDocument"]["uri"]), false, true);
    return;
  }

  for (auto& change : request["params"]["contentChanges"]) {
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
    // A range and text was provided
    if (start == end) {
      std::string preLine = std::string(newDoc[start]);
      newDoc[start] = preLine.substr(0, startChar) + lines[0] + preLine.substr(endChar);
    } else {
      newDoc[start] = newDoc[start].substr(0, startChar) + lines[0];
      for (size_t i = 1; i < lines.size() - 1; ++i) {
        newDoc.insert(newDoc.begin() + start + i, lines[i]);
      }
      newDoc[start + lines.size() - 1] = lines.back() + newDoc[end].substr(endChar);
      if (end - start > 1) {
        newDoc.erase(newDoc.begin() + start + lines.size(), newDoc.begin() + end + 1);
      }
    }
    std::string result;
    for (size_t i = 0; i < newDoc.size(); i++) {
      result += newDoc[i];
      if (i != newDoc.size() - 1) result += "\n";
    };
    if (result.empty()) result = "\n";
    documents[uri] = result;
  }
  // This is probably fine
  TypeChecker::performCheck(Parser::parse(lsp::document::getText(request["params"]["textDocument"]["uri"]).c_str(), request["params"]["textDocument"]["uri"]), false, true);
}

std::string lsp::document::getText(std::string uri) {
  if (!documents.contains(uri)) return "";
  return documents[uri];
};