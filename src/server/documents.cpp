#include <unordered_map>
#include "../parser/parser.hpp"
#include "../codegen/gen.hpp"
#include "logging.hpp"
#include "lsp.hpp"

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
  Word result;

  auto it = documents.find(uri);
  if (it == documents.end()) {
    logging::log("URI NOT FOUND\n");
    return result; // URI not found (return null)
  }

  const std::string& document = it->second;
  std::vector<std::string> lines;
  size_t start = 0, end;
  while ((end = document.find('\n', start)) != std::string::npos) {
      lines.push_back(document.substr(start, end - start));
      start = end + 1;
  }
  lines.push_back(document.substr(start));

  if (pos.line >= lines.size()) return result;
  const std::string& line = lines[pos.line];
  if (pos.character > line.size()) return result;

  // Scan left and right for alphanumeric or underscore characters
  unsigned long long left = pos.character;
  while (left > 0 && (std::isalnum(line[left - 1]) || line[left - 1] == '_' || line[left - 1] == '@')) {
      --left;
  }

  unsigned long long right = pos.character;
  while (right < line.size() && (std::isalnum(line[right]) || line[right] == '_')) {
      ++right;
  }

  result.range.start = { pos.line, left };
  result.range.end = { pos.line, right };
  result.word = line.substr(left, right - left);

  return result;
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
  // The uri should be absolute. However, it starts with "file://"
  std::string parserURI = request["params"]["textDocument"]["uri"];
  // Add this to the codegen file ids
  if (parserURI.starts_with("file://")) {
    parserURI = parserURI.substr(7);
  }
  codegen::fileIDs.push_back(parserURI);
  TypeChecker::performCheck(Parser::parse(contents.c_str(), parserURI), false, true);
  if (contents.empty()) {
    documents.emplace(uri, "\n"); // the newline is very important here
  } else {
    documents.emplace(uri, contents);
  }
};

void lsp::events::documentChange(nlohmann::json& request) {
  const std::string &currentDocument = documents[request["params"]["textDocument"]["uri"]];
  std::string newDocument = currentDocument;

    // LSP textDocument/didChange sends an array of content changes
    if (!request.contains("contentChanges") || !request["contentChanges"].is_array()) {
      return; // Nothing changed. No need to update the document.
    }

    for (const auto& change : request["contentChanges"]) {
        if (change.contains("range")) {
            // Incremental change
            unsigned long long int startLine = change["range"]["start"]["line"];
            unsigned long long int startChar = change["range"]["start"]["character"];
            unsigned long long int endLine = change["range"]["end"]["line"];
            unsigned long long int endChar = change["range"]["end"]["character"];
            std::string newText = change["text"];

            // Split the document into lines
            std::vector<std::string> lines;
            size_t start = 0, end;
            while ((end = newDocument.find('\n', start)) != std::string::npos) {
                lines.push_back(newDocument.substr(start, end - start));
                start = end + 1;
            }
            // Add the last line if it doesn't end in newline
            lines.push_back(newDocument.substr(start));

            // Handle edge cases
            if (startLine >= lines.size()) startLine = lines.size() - 1;
            if (endLine >= lines.size()) endLine = lines.size() - 1;

            // Modify the text
            std::string& startLineText = lines[startLine];
            std::string& endLineText = lines[endLine];

            std::string before = startLineText.substr(0, startChar);
            std::string after = endLineText.substr(endChar);

            std::string replacedText = before + newText + after;

            // Replace lines from startLine to endLine
            lines.erase(lines.begin() + startLine, lines.begin() + endLine + 1);
            lines.insert(lines.begin() + startLine, replacedText);

            // Rebuild document
            newDocument.clear();
            for (size_t i = 0; i < lines.size(); ++i) {
                newDocument += lines[i];
                if (i + 1 < lines.size()) newDocument += '\n';
            }
        } else if (change.contains("text")) {
            // Full text replacement (though we assume incremental, just in case)
            newDocument = change["text"];
        }
    }

    documents[request["params"]["textDocument"]["uri"]] = newDocument;
    TypeChecker::performCheck(Parser::parse(newDocument.c_str(), request["params"]["textDocument"]["uri"]), false, true);
    return;
}

std::string lsp::document::getText(std::string uri) {
  if (!documents.contains(uri)) return "";
  return documents[uri];
};