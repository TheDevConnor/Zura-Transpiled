#include "../ast/stmt.hpp"

#include <filesystem>
#include <vector>

#include "../ast/ast.hpp"
#include "../codegen/gen.hpp"
#include "../helper/error/error.hpp"
#include "../helper/flags.hpp"
#include "parser.hpp"

Node::Stmt *Parser::parseStmt(PStruct *psr, std::string name) {
  Node::Stmt *stmt_it = stmt(psr, name);

  if (stmt_it != nullptr)
    return stmt_it;

  return exprStmt(psr);
}

Node::Stmt *Parser::exprStmt(PStruct *psr) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  Node::Expr *expr = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::SEMICOLON, "Expected a SEMICOLON after an expr stmt");
  return new ExprStmt(line, column, expr,
                      codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::blockStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a block stmt");
  std::vector<Node::Stmt *> stmts;
  std::vector<Node::Type *> varDeclTypes;
  bool shouldDeclareBackwards = true;
  while (psr->current().kind != TokenKind::RIGHT_BRACE) {
    stmts.push_back(parseStmt(psr, name));
    // Check if the latest statemnt was a return statement
    if (stmts.back()->kind == NodeKind::ND_RETURN_STMT) {
      break; // Why continue? We can't get any further, anyway
    }

    // Check if the latest statement was a variable declaration
    if (stmts.back()->kind == NodeKind::ND_VAR_STMT && shouldDeclareBackwards) {
      VarStmt *var = static_cast<VarStmt *>(stmts.back());
      if (var->isConst)
        continue; // Const variables don't get pushed back on the stack

      // Push back the size of the variable to the block statement's size
      varDeclTypes.push_back(var->type);
    }
    NodeKind pastKind = stmts.back()->kind;
    if (pastKind == NodeKind::ND_IF_STMT ||
        pastKind == NodeKind::ND_WHILE_STMT ||
        pastKind == NodeKind::ND_FOR_STMT ||
        pastKind == NodeKind::ND_BLOCK_STMT ||
        pastKind == NodeKind::ND_FN_STMT) {
      // This means that we have nested scopes
      // So in that case, we should declare variables forward
      // Fuck you, C. Why do you do it this way?
      shouldDeclareBackwards = false;
      varDeclTypes.clear();
    }
  }

  psr->expect(TokenKind::RIGHT_BRACE, "Expected a R_BRACE to end a block stmt");
  return new BlockStmt(line, column, stmts, !shouldDeclareBackwards,
                       varDeclTypes, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::varStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  bool isConst = psr->current().kind == TokenKind::_CONST;
  psr->expect(TokenKind::VAR,
              "Expected a VAR or CONST keyword to start a var stmt");

  name = psr->expect(TokenKind::IDENTIFIER,
                     "Expected an IDENTIFIER after a VAR or CONST keyword")
             .value;

  psr->expect(TokenKind::COLON, "Expected a COLON after the variable name in a "
                                "var stmt to define the type of the variable");
  Node::Type *varType = parseType(psr);

  if (psr->current().kind == TokenKind::SEMICOLON) {
    psr->expect(TokenKind::SEMICOLON,
                "Expected a SEMICOLON at the end of a var stmt");
    return new VarStmt(line, column, isConst, name, varType, nullptr,
                       codegen::getFileID(psr->current_file));
  }

  psr->expect(TokenKind::EQUAL,
              "Expected a EQUAL after the type of the variable in a var stmt");
  Node::Expr *assignedValue = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a var stmt");

  return new VarStmt(line, column, isConst, name, varType, assignedValue,
                     codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::printStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::PRINT,
              "Expected a PRINT keyword to start an output stmt");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start an output stmt");

  Node::Expr *fileDescriptor = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the file descriptor in an output stmt");

  std::vector<Node::Expr *> args; // Change the type of args vector

  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, BindingPower::defaultValue));
    if (psr->current().kind == RIGHT_PAREN)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after an arguement in an output stmt");
  }

  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end an output stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an output stmt");

  return new OutputStmt(line, column, fileDescriptor, args,
                        codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::printlnStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::PRINTLN,
              "Expected a PRINTLN keyword to start an output stmt");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start an output stmt");

  Node::Expr *fileDescriptor = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the file descriptor in an output stmt");

  std::vector<Node::Expr *> args; // Change the type of args vector
  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, BindingPower::defaultValue));
    if (psr->current().kind == RIGHT_PAREN)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after an arguement in an output stmt");
  }

  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end an output stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an output stmt");

  return new OutputStmt(line, column, fileDescriptor, args,
                        codegen::getFileID(psr->current_file), true);
}

Node::Stmt *Parser::constStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::_CONST,
              "Expected a CONST keyword to start a const stmt");
  name = psr->expect(TokenKind::IDENTIFIER,
                     "Expected an IDENTIFIER after a CONST keyword")
             .value;

  psr->expect(TokenKind::WALRUS,
              "Expected a WALRUS after the variable name in a const stmt");

  Node::Stmt *value = parseStmt(psr, name);

  return new ConstStmt(line, column, name, value,
                       codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::funStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::FUN,
              "Expected a FUN keyword to start a function stmt");

  // Check for a template function fn <typnames> (params) -> returnType
  bool isTemplate = false;
  std::vector<std::string> typenames;
  if (psr->current().kind == TokenKind::LESS) {
    isTemplate = true;
    psr->expect(TokenKind::LESS,
                "Expected a LESS to start a template function");
    while (psr->current().kind != TokenKind::GREATER) {
      // Expect identifiers and commas
      std::string typeName =
          psr->expect(TokenKind::IDENTIFIER,
                      "Expected an IDENTIFIER as a template type name")
              .value;
      typenames.push_back(typeName);
      if (psr->current().kind == TokenKind::GREATER)
        break;
      psr->expect(TokenKind::COMMA,
                  "Expected a COMMA after a template type name");
    }
    psr->expect(TokenKind::GREATER,
                "Expected a GREATER to end the template function");
  }

  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a function stmt");

  std::vector<std::pair<std::string, Node::Type *>> params;
  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    std::string paramName =
        psr->expect(
               TokenKind::IDENTIFIER,
               "Expected an IDENTIFIER as a parameter name in a function stmt")
            .value;
    psr->expect(TokenKind::COLON,
                "Expected a COLON after the parameter name in a function stmt");
    Node::Type *paramType = parseType(psr);
    if (paramType == nullptr)
      Error::handle_error("Parser", psr->current_file,
                          "Expected a type for the parameter", psr->tks,
                          psr->current().line, psr->current().column);
    params.push_back({paramName, paramType});

    if (psr->current().kind == TokenKind::RIGHT_PAREN)
      break;
    psr->expect(TokenKind::COMMA,
                "Expected a COMMA after a parameter in a function stmt");
  }
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end the parameters in a function stmt");

  Node::Type *returnType = parseType(psr);
  if (returnType == nullptr)
    Error::handle_error("Parser", psr->current_file,
                        "Expected a type for the return type", psr->tks,
                        psr->current().line, psr->current().column);

  Node::Stmt *body = parseStmt(psr, name);
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a function stmt");
  if (isTemplate)
    return new FnStmt(line, column, name, params, returnType, body, typenames,
                      true, false, true, codegen::getFileID(psr->current_file));
  if (name == "main")
    return new FnStmt(line, column, name, params, returnType, body, typenames,
                      true, true, false, codegen::getFileID(psr->current_file));
  return new FnStmt(line, column, name, params, returnType, body, typenames,
                    false, false, false, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::returnStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::RETURN,
              "Expected a RETURN keyword to start a return stmt");
  if (psr->peek().kind == TokenKind::SEMICOLON) {
    psr->advance();
    return new ReturnStmt(line, column, nullptr,
                          codegen::getFileID(psr->current_file));
  }
  Node::Expr *expr = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a return stmt");
  return new ReturnStmt(line, column, expr,
                        codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::ifStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::IF, "Expected an IF keyword to start an if stmt");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start the condition in an if stmt");
  Node::Expr *condition = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end the condition in an if stmt");

  Node::Stmt *thenStmt = parseStmt(psr, name);
  Node::Stmt *elseStmt = nullptr;

  if (psr->current().kind == TokenKind::ELSE) {
    psr->expect(TokenKind::ELSE,
                "Expected an ELSE keyword to start the else stmt");
    elseStmt = parseStmt(psr, name);
  }

  return new IfStmt(line, column, condition, thenStmt, elseStmt,
                    codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::structStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::STRUCT,
              "Expected a STRUCT keyword to start a struct stmt");

  bool isTemplate = false;
  std::vector<std::string> typenames;
  if (psr->current().kind == LESS) {
    isTemplate = true;
    psr->expect(TokenKind::LESS, "Expected a LESS to start a template struct");
    while (psr->current().kind != TokenKind::GREATER) {
      std::string typeName =
          psr->expect(TokenKind::IDENTIFIER,
                      "Expected an IDENTIFIER as a template type name")
              .value;
      typenames.push_back(typeName);
      if (psr->current().kind == TokenKind::GREATER)
        break;
      psr->expect(TokenKind::COMMA,
                  "Expected a COMMA to seperate the typenames");
    }
    psr->expect(TokenKind::GREATER,
                "Expected a GREATER to end the template struct");
  }

  psr->expect(TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a struct stmt");

  std::vector<std::pair<std::string, Node::Type *>> fields;
  std::vector<Node::Stmt *> stmts;
  bool warnForSemi = true;

  while (psr->current().kind != TokenKind::RIGHT_BRACE) {
    TokenKind tokenKind = psr->current().kind;
    switch (tokenKind) {
    case TokenKind::IDENTIFIER: {
      std::string fieldName =
          psr->expect(TokenKind::IDENTIFIER,
                      "Expected an IDENTIFIER as a field name in a struct stmt")
              .value;

      // Check if the field is a fn, enum, struct, or union
      if (psr->current().kind == TokenKind::WALRUS) {
        // Parse as if it is an actual function
        psr->advance(); // Consume detected :=
        switch (psr->current().kind) {
        case TokenKind::FUN:
          stmts.push_back(funStmt(psr, fieldName));
          break;
        default:
          std::string msg =
              "Structs only take in fields, structs, and functions. ";
          msg += "Found unexpected token: " + psr->current().value;
          Error::handle_error("Parser", psr->current_file, msg, psr->tks,
                              psr->current().line, psr->current().column);
          break;
        }
        break;
      }

      psr->expect(TokenKind::COLON,
                  "Expected a COLON after the field name in a struct stmt");
      Node::Type *fieldType = parseType(psr);
      fields.push_back({fieldName, fieldType});
      if (psr->peek().kind == TokenKind::RIGHT_BRACE)
        break; // who cares about the semicolon/comma?
      if (warnForSemi) {
        if (psr->current().kind == TokenKind::SEMICOLON) {
          // Warn that semicolons are not standard, but do not exit the program; allow compilation as normal
          Error::handle_error("Parser", psr->current_file, "Semicolons are not standard in struct field lists; use commas instead",
                              psr->tks, psr->current().line, psr->current().column, true);
        } else
          psr->expect(TokenKind::COMMA, "Expected a COMMA after a struct field, "
                                        "instead got a " + std::string(Lexer::tokenToStringMap[psr->current().kind]));
      } else {
        if (psr->current().kind == TokenKind::COMMA) {
          psr->advance();
        }
        psr->expect(TokenKind::SEMICOLON,
                    "Expected a comma or a semicolon for struct field list");
      }
      break;
    }
    case TokenKind::SEMICOLON: {
      psr->expect(TokenKind::SEMICOLON,
                  "Expected a SEMICOLON after a field in a struct stmt");
      break;
    }
    case TokenKind::_CONST:
      stmts.push_back(constStmt(psr, name));
      break;
    default:
      stmts.push_back(parseStmt(psr, name));
      break;
    }
  }

  psr->expect(TokenKind::RIGHT_BRACE,
              "Expected a R_BRACE to end a struct stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a struct stmt");

  if (stmts.size() > 0)
    return new StructStmt(line, column, name, fields, stmts, typenames,
                          codegen::getFileID(psr->current_file), false);
  if (isTemplate)
    return new StructStmt(line, column, name, fields, stmts, typenames,
                          codegen::getFileID(psr->current_file), true);
  return new StructStmt(line, column, name, fields, stmts, typenames,
                        codegen::getFileID(psr->current_file), false);
}

Node::Stmt *Parser::loopStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::LOOP, "Expected a LOOP keyword to start a loop stmt");
  psr->expect(TokenKind::LEFT_PAREN, "Expected a L_PAREN to start a loop stmt");

  std::string varName;
  Node::Expr *forLoop = nullptr;
  Node::Expr *condition = nullptr;
  Node::Expr *whileLoop = nullptr;
  Node::Expr *opCondition = nullptr;
  bool isForLoop = false;
  bool isOptional = false;

  while (psr->current().kind != TokenKind::RIGHT_PAREN) {
    // First condition is the for loop condition
    // loop (i = 0; i < 10) : (++1)
    // Second condition is the while loop condition
    // loop (i < 10) : (++1)

    // set the current token which is the variable name in the for loop
    varName = psr->current().value;

    // we need to look two tokens ahead to determine if it is a for loop or a
    // while loop
    if (psr->peek(1).kind == TokenKind::EQUAL) {
      isForLoop = true;
      forLoop = parseExpr(psr, BindingPower::defaultValue);
      psr->expect(
          TokenKind::SEMICOLON,
          "Expected a SEMICOLON after the for loop initializer in a loop stmt");
      condition = parseExpr(psr, BindingPower::defaultValue);
    } else {
      whileLoop = parseExpr(psr, BindingPower::defaultValue);
    }

    psr->expect(TokenKind::RIGHT_PAREN,
                "Expected a R_PAREN to end the condition in a loop stmt");
    if (psr->current().kind == TokenKind::COLON) {
      isOptional = true;
      psr->expect(TokenKind::COLON,
                  "Expected a COLON to start the body of a loop stmt");
      psr->expect(TokenKind::LEFT_PAREN,
                  "Expected a L_PAREN to start the condition in a loop stmt");
      opCondition = parseExpr(psr, BindingPower::defaultValue);
      psr->expect(TokenKind::RIGHT_PAREN,
                  "Expected a R_PAREN to end the condition in a loop stmt");
    }

    Node::Stmt *body = parseStmt(psr, name);
    // Check if there is a semicolon after the block. If there is, we can
    // consume it, no problem. Semicolon here is optional
    if (psr->current().kind == TokenKind::SEMICOLON)
      psr->advance();
    if (isOptional) {
      if (isForLoop)
        return new ForStmt(line, column, varName, forLoop, condition,
                           opCondition, body,
                           codegen::getFileID(psr->current_file));
      return new WhileStmt(line, column, whileLoop, opCondition, body,
                           codegen::getFileID(psr->current_file));
    }
    if (isForLoop)
      return new ForStmt(line, column, varName, forLoop, condition, nullptr,
                         body, codegen::getFileID(psr->current_file));
    return new WhileStmt(line, column, whileLoop, nullptr, body,
                         codegen::getFileID(psr->current_file));
  }
  return nullptr;
}

Node::Stmt *Parser::matchStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::MATCH,
              "Expected a MATCH keyword to start a match stmt");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a match stmt catch");
  Node::Expr *cond = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end the condition in a match stmt");
  psr->expect(TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a match stmt body");

  std::vector<std::pair<Node::Expr *, Node::Stmt *>> cases;
  Node::Stmt *defaultCase = nullptr;
  while (psr->current().kind != TokenKind::RIGHT_BRACE) {
    if (psr->current().kind == TokenKind::DEFAULT) {
      psr->advance();
      psr->expect(
          TokenKind::RIGHT_ARROW,
          "Expected a RIGHT_ARROW after the DEFAULT keyword in a match stmt");
      defaultCase =
          blockStmt(psr, name); // Will automatically detect another L_BRACE
      // This expects a R_BRACE, but the programmer may have made the LINGUISTIC
      // choice to include a semicolon
      if (psr->current().kind == TokenKind::SEMICOLON)
        psr->advance();
      continue;
    }
    if (psr->current().kind == TokenKind::CASE) {
      psr->advance();
      Node::Expr *caseExpr = parseExpr(psr, BindingPower::defaultValue);
      psr->expect(
          TokenKind::RIGHT_ARROW,
          "Expected a RIGHT_ARROW after the case expression in a match stmt");
      Node::Stmt *caseStmt = blockStmt(psr, name);
      cases.push_back({caseExpr, caseStmt});
      // semicolon expectancy
      if (psr->current().kind == TokenKind::SEMICOLON)
        psr->advance();
    } else {
      // What the hell token is this?
      // Not creating a case here would result in an infinite loop.
      // This is a safety measure to prevent that.
      std::string msg =
          "Unexpected token in match statement: " + psr->current().value;
      Error::handle_error("Parser", psr->current_file, msg, psr->tks,
                          psr->current().line, psr->current().column);
      psr->advance(); // Consume the bad token
    }
  }

  psr->expect(TokenKind::RIGHT_BRACE, "Expected a R_BRACE to end a match stmt");
  return new MatchStmt(line, column, cond, cases, defaultCase,
                       codegen::getFileID(psr->current_file));
};

Node::Stmt *Parser::enumStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(TokenKind::ENUM,
              "Expected an ENUM keyword to start an enum stmt");
  psr->expect(TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start an enum stmt");

  std::vector<std::string> fields;
  bool warnForSemi = true;
  while (psr->current().kind != TokenKind::RIGHT_BRACE) {
    Lexer::Token ident =
        psr->expect(TokenKind::IDENTIFIER,
                    "Expected an IDENTIFIER as a field name in an enum stmt");
    if (ident.kind != TokenKind::IDENTIFIER)
      break; // empty the error accumulator
    fields.push_back(ident.value);
    // Check if next character is brace - comma not REQUIRED there
    if (psr->current().kind == TokenKind::RIGHT_BRACE)
      break;
    if (psr->current().kind == TokenKind::SEMICOLON) {
      // that's fine too, but warn
      Lexer::Token semi = psr->advance();
      if (warnForSemi) {
        Error::handle_error("Parser", psr->current_file,
                            "Semicolons are non-standard for enumerator lists",
                            psr->tks, semi.line, semi.column);
        warnForSemi = false; // only warn once to stop console from filling with
                             // all the same error (especially when the
                             // programmer only made a simple mistake)
      }
    } else
      psr->expect(TokenKind::COMMA,
                  "Expected a COMMA after a field name in an enum stmt");
  }
  psr->expect(TokenKind::RIGHT_BRACE, "Expected a R_BRACE to end an enum stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an enum stmt");

  return new EnumStmt(line, column, name, fields,
                      codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::importStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  // Declare our file as first. If we are main but the first thing we do is import another file,
  // this will maintain the first file, as, well, first.
  (void)codegen::getFileID(psr->current_file);

  psr->expect(TokenKind::IMPORT,
              "Expected an IMPORT keyword to start an import stmt");
  std::string current_file = node.current_file;
  std::string path =
      psr->expect(TokenKind::STRING,
                  "Expected a STRING as a path in an import stmt")
          .value;
  node.current_file = path;
  psr->current_file = path;

  path = path.substr(1, path.size() - 2); // removes "" from the path

  // Make an absolute path to the imported file
  // that is relative to the file that called it
  // ie: if the file that called it is in /home/user/file1
  // and the imported file is in /home/user/std/file2
  // the absolute path will be /home/user/std/file2

  // Check if the path is already absolute
  if (path.starts_with("file://") || path.starts_with("/"))
    path = path.substr(7);
  std::filesystem::path absolutePath = path;
  if (absolutePath.is_relative()) {
    absolutePath = std::filesystem::absolute(
        std::filesystem::path(current_file).parent_path() / path);
  }
  char *fileContent = Flags::readFile(absolutePath.string().c_str());
  Node::Stmt *result = parse(fileContent, absolutePath.string().c_str());
  if (result == nullptr) {
    Error::handle_error("Parser", psr->current_file,
                        "Could not parse the imported file '" + path + "'",
                        psr->tks, line, column);
    return nullptr;
  }

  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an import stmt");

  node.current_file = current_file;
  psr->current_file = current_file;

  return new ImportStmt(line, column, absolutePath, result,
                        codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::linkStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  // @link "path";
  psr->expect(TokenKind::LINK, "Expected a LINK keyword to start a link stmt");
  std::string path = psr->expect(TokenKind::STRING,
                                 "Expected a STRING as a path in a link stmt")
                         .value;
  path.erase(path.begin());   // Erase initial "
  path.erase(path.end() - 1); // Erase final "
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a link stmt");
  return new LinkStmt(line, column, path,
                      codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::externStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  std::vector<std::string> externs;

  // @extern <"", "", "">; or @extern "C";
  psr->expect(TokenKind::EXTERN,
              "Expected an EXTERN keyword to start an extern stmt");
  if (psr->current().kind == TokenKind::LESS) {
    psr->expect(TokenKind::LESS, "Expected a LESS to start an extern stmt");
    while (psr->current().kind != TokenKind::GREATER) {
      std::string path =
          psr->expect(TokenKind::STRING,
                      "Expected a STRING as a path in an extern stmt")
              .value;
      path.erase(path.begin());   // Erase initial "
      path.erase(path.end() - 1); // Erase final "
      externs.push_back(path);
      if (psr->current().kind == TokenKind::GREATER)
        break;
      psr->expect(TokenKind::COMMA,
                  "Expected a COMMA after a path in an extern stmt");
    }
    psr->expect(TokenKind::GREATER, "Expected a GREATER to end an extern stmt");
    psr->expect(TokenKind::SEMICOLON,
                "Expected a SEMICOLON at the end of an extern stmt");
    return new ExternStmt(line, column, "", externs,
                          codegen::getFileID(psr->current_file));
  }
  std::string path =
      psr->expect(TokenKind::STRING,
                  "Expected a STRING as a path in an extern stmt")
          .value;
  path.erase(path.begin());   // Erase initial "
  path.erase(path.end() - 1); // Erase final "
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an extern stmt");
  return new ExternStmt(line, column, path, externs,
                        codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::breakStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::BREAK,
              "Expected a BREAK keyword to start a break stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a break stmt");
  return new BreakStmt(line, column, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::continueStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::CONTINUE,
              "Expected a CONTINUE keyword to start a continue stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a continue stmt");
  return new ContinueStmt(line, column, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::inputStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::INPUT,
              "Expected an INPUT keyword to start an input stmt");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start an input stmt");

  Node::Expr *fileDescriptor = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the file descriptor in an input stmt");

  Node::Expr *bufferOut = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::COMMA,
              "Expected a COMMA after the variable in an input stmt");

  Node::Expr *maxBytes = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end an input stmt");
  psr->expect(TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an input stmt");
  return new InputStmt(line, column, fileDescriptor, bufferOut, maxBytes,
                       codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::closeStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  (void)name; // mark it as unused

  psr->expect(TokenKind::CLOSE,
              "Expected a CLOSE keyword to start a close stmt");
  psr->expect(TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a close stmt");
  Node::Expr *fd = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end a close stmt");
  psr->expect(
      TokenKind::SEMICOLON,
      "Expected a SEMICOLON at the end of a close stmt"); // This wouldn't
                                                          // really return
                                                          // anything
  return new CloseStmt(line, column, fd, codegen::getFileID(psr->current_file));
}
