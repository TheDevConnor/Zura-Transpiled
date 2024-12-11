#include "../ast/stmt.hpp"
#include "../ast/ast.hpp"
#include "../helper/flags.hpp"
#include "../codegen/gen.hpp"
#include "../typeChecker/type.hpp"
#include "parser.hpp"
#include <filesystem>
#include <vector>

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
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a expr stmt!");
  return new ExprStmt(line, column, expr, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::blockStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a block stmt");
  std::vector<Node::Stmt *> stmts;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
    stmts.push_back(parseStmt(psr, name));
  }

  psr->expect(psr, TokenKind::RIGHT_BRACE,
              "Expected a R_BRACE to end a block stmt");
  return new BlockStmt(line, column, stmts, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::varStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  bool isConst = psr->current(psr).kind == TokenKind::_CONST;
  psr->expect(psr, TokenKind::VAR,
              "Expected a VAR or CONST keyword to start a var stmt");

  name = psr->expect(psr, TokenKind::IDENTIFIER,
                     "Expected an IDENTIFIER after a VAR or CONST keyword")
             .value;

  psr->expect(psr, TokenKind::COLON,
              "Expected a COLON after the variable name in a var stmt to "
              "define the type of the variable");
  Node::Type *varType = parseType(psr, BindingPower::defaultValue);

  if (psr->current(psr).kind == TokenKind::SEMICOLON) {
    psr->expect(psr, TokenKind::SEMICOLON,
                "Expected a SEMICOLON at the end of a var stmt");
    return new VarStmt(line, column, isConst, name, varType, nullptr, codegen::getFileID(psr->current_file));
  }

  psr->expect(psr, TokenKind::EQUAL,
              "Expected a EQUAL after the type of the variable in a var stmt");
  Node::Expr *assignedValue = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a var stmt");
  int currFileID = codegen::getFileID(psr->current_file);
  return new VarStmt(line, column, isConst, name, varType,
                     assignedValue, currFileID);
}

Node::Stmt *Parser::printStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::PRINT,
              "Expected a PRINT keyword to start a print stmt");
  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a print stmt");

  std::vector<Node::Expr *> args; // Change the type of args vector

  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    args.push_back(parseExpr(psr, BindingPower::defaultValue));
    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after an arguement in a print stmt");
  }

  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end a print stmt");
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a print stmt");

  return new PrintStmt(line, column, args, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::constStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::_CONST,
              "Expected a CONST keyword to start a const stmt");
  name = psr->expect(psr, TokenKind::IDENTIFIER,
                     "Expected an IDENTIFIER after a CONST keyword")
             .value;

  psr->expect(psr, TokenKind::WALRUS,
              "Expected a WALRUS after the variable name in a const stmt");

  Node::Stmt *value = parseStmt(psr, name);

  return new ConstStmt(line, column, name, value, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::funStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::FUN,
              "Expected a FUN keyword to start a function stmt");

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a function stmt");
  std::vector<std::pair<std::string, Node::Type *>> params;
  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    std::string paramName =
        psr->expect(
               psr, TokenKind::IDENTIFIER,
               "Expected an IDENTIFIER as a parameter name in a function stmt")
            .value;
    psr->expect(psr, TokenKind::COLON,
                "Expected a COLON after the parameter name in a function stmt");
    Node::Type *paramType = parseType(psr, BindingPower::defaultValue);
    params.push_back({paramName, paramType});

    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after a parameter in a function stmt");
  }
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end the parameters in a function stmt");

  Node::Type *returnType = parseType(psr, BindingPower::defaultValue);

  Node::Stmt *body = parseStmt(psr, name);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a function stmt");

  if (name == "main")
    return new FnStmt(line, column, name, params, returnType, body, true, true, codegen::getFileID(psr->current_file));
  return new FnStmt(line, column, name, params, returnType, body, false, false, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::returnStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::RETURN,
              "Expected a RETURN keyword to start a return stmt");
  if (psr->peek(psr).kind == TokenKind::SEMICOLON) {
    psr->advance(psr);
    return new ReturnStmt(line, column, nullptr, codegen::getFileID(psr->current_file));
  }
  Node::Expr *expr = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a return stmt");
  return new ReturnStmt(line, column, expr, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::ifStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::IF, "Expected an IF keyword to start an if stmt");
  Node::Expr *condition = parseExpr(psr, BindingPower::defaultValue);

  Node::Stmt *thenStmt = parseStmt(psr, name);
  Node::Stmt *elseStmt = nullptr;

  if (psr->current(psr).kind == TokenKind::ELSE) {
    psr->expect(psr, TokenKind::ELSE,
                "Expected an ELSE keyword to start the else stmt");
    elseStmt = parseStmt(psr, name);
  }

  return new IfStmt(line, column, condition, thenStmt, elseStmt, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::structStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::STRUCT,
              "Expected a STRUCT keyword to start a struct stmt");

  psr->expect(psr, TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a struct stmt");

  std::vector<std::pair<std::string, Node::Type *>> fields;
  std::vector<Node::Stmt *> stmts;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
    TokenKind tokenKind = psr->current(psr).kind;
    switch (tokenKind) {
    case TokenKind::IDENTIFIER: {
      std::string fieldName =
          psr->expect(psr, TokenKind::IDENTIFIER,
                      "Expected an IDENTIFIER as a field name in a struct stmt")
              .value;

      // Check if the field is a fn, enum, struct, or union
      if (psr->current(psr).kind == TokenKind::WALRUS) {
        // Parse as if it is an actual function
        psr->advance(psr); // Consume detected :=
        switch (psr->current(psr).kind) {
        case TokenKind::FUN:
          stmts.push_back(funStmt(psr, fieldName));
          break; 
        default:
          std::string msg = "Structs only take in fields, structs, and functions. ";
          msg += "Found unexpected token: " + psr->current(psr).value;
          ErrorClass::error(line, column, msg, "", "Parser Error",
                            psr->current_file.c_str(), lexer, psr->tks, true,
                            false, false, false, false, false);
          break;
        }
        break;
      }
      
      psr->expect(psr, TokenKind::COLON,
                  "Expected a COLON after the field name in a struct stmt");
      Node::Type *fieldType = parseType(psr, BindingPower::defaultValue);
      fields.push_back({fieldName, fieldType});
      psr->expect(psr, TokenKind::SEMICOLON,
                  "Expected a SEMICOLON after the field type in a struct stmt");
      break;
    }
    case TokenKind::SEMICOLON: {
      psr->expect(psr, TokenKind::SEMICOLON,
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

  psr->expect(psr, TokenKind::RIGHT_BRACE,
              "Expected a R_BRACE to end a struct stmt");
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a struct stmt");

  if (stmts.size() > 0)
    return new StructStmt(line, column, name, fields, stmts, codegen::getFileID(psr->current_file));
  return new StructStmt(line, column, name, fields, {}, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::loopStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LOOP,
              "Expected a LOOP keyword to start a loop stmt");

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a loop stmt");

  std::string varName;
  Node::Expr *forLoop;
  Node::Expr *condition;
  Node::Expr *whileLoop;
  Node::Expr *opCondition;
  bool isForLoop = false;
  bool isOptional = false;

  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    // First condition is the for loop condition
    // loop (i = 0; i < 10) : (++1)
    // Second condition is the while loop condition
    // loop (i < 10) : (++1)

    // set the current token which is the variable name in the for loop
    varName = psr->current(psr).value;

    // we need to look two tokens ahead to determine if it is a for loop or a
    // while loop
    if (psr->peek(psr, 1).kind == TokenKind::EQUAL) {
      isForLoop = true;
      forLoop = parseExpr(psr, BindingPower::defaultValue);

      psr->expect(
          psr, TokenKind::SEMICOLON,
          "Expected a SEMICOLON after the for loop initializer in a loop stmt");

      condition = parseExpr(psr, BindingPower::defaultValue);

    } else {
      whileLoop = parseExpr(psr, BindingPower::defaultValue);
    }

    psr->expect(psr, TokenKind::RIGHT_PAREN,
                "Expected a R_PAREN to end the condition in a loop stmt");

    if (psr->current(psr).kind == TokenKind::COLON) {
      isOptional = true;
      psr->expect(psr, TokenKind::COLON,
                  "Expected a COLON to start the body of a loop stmt");
      psr->expect(psr, TokenKind::LEFT_PAREN,
                  "Expected a L_PAREN to start the condition in a loop stmt");
      opCondition = parseExpr(psr, BindingPower::defaultValue);
      psr->expect(psr, TokenKind::RIGHT_PAREN,
                  "Expected a R_PAREN to end the condition in a loop stmt");
    }

    Node::Stmt *body = parseStmt(psr, name);

    if (isOptional) {
      if (isForLoop)
        return new ForStmt(line, column, varName, forLoop, condition,
                           opCondition, body, codegen::getFileID(psr->current_file));
      return new WhileStmt(line, column, whileLoop, opCondition, body, codegen::getFileID(psr->current_file));
    }
    if (isForLoop)
      return new ForStmt(line, column, varName, forLoop, condition, nullptr,
                         body, codegen::getFileID(psr->current_file));
    return new WhileStmt(line, column, whileLoop, nullptr, body, codegen::getFileID(psr->current_file));
  }

  return nullptr;
}

Node::Stmt *Parser::enumStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::ENUM,
              "Expected an ENUM keyword to start an enum stmt");
  psr->expect(psr, TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start an enum stmt");

  std::vector<std::string> fields;
  while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
    fields.push_back(
        psr->expect(psr, TokenKind::IDENTIFIER,
                    "Expected an IDENTIFIER as a field name in an enum stmt")
            .value);
    // Check if next character is brace - comma not REQUIRED there
    if (psr->current(psr).kind == TokenKind::RIGHT_BRACE)
      break;
    psr->expect(psr, TokenKind::COMMA,
                "Expected a COMMA after a field in an enum stmt");
  }
  psr->expect(psr, TokenKind::RIGHT_BRACE,
              "Expected a R_BRACE to end an enum stmt");
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an enum stmt");

  return new EnumStmt(line, column, name, fields, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::templateStmt(PStruct *psr, std::string name) {
  // template <typealias T>
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::TEMPLATE,
              "Expected a TEMPLATE keyword to start a template stmt");

  psr->expect(psr, TokenKind::LESS, "Expected a '<' to start a template stmt");

  std::vector<std::string> typeParams;
  while (psr->current(psr).kind != TokenKind::GREATER) {
    psr->expect(
        psr, TokenKind::TYPEALIAS,
        "Expected a TYPEALIAS keyword as a type parameter in a template "
        "stmt");
    typeParams.push_back(psr->expect(psr, TokenKind::IDENTIFIER,
                                     "Expected an IDENTIFIER as a type "
                                     "parameter in a template stmt")
                             .value);
    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after a type parameter in a template stmt");
  }

  psr->expect(psr, TokenKind::GREATER, "Expected a '>' to end a template stmt");

  return new TemplateStmt(typeParams, line, column, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::importStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;


  psr->expect(psr, TokenKind::IMPORT,
              "Expected an IMPORT keyword to start an import stmt");

  std::string current_file = node.current_file;
  std::string path = psr->expect(psr, TokenKind::STRING,
                          "Expected a STRING as a path in an import stmt")
                  .value;
  node.current_file = path;

  path = path.substr(1, path.size() - 2); // removes "" from the path

  // Make an absolute path to the imported file
  // that is relative to the file that called it
  // ie: if the file that called it is in /home/user/file1
  // and the imported file is in /home/user/std/file2
  // the absolute path will be /home/user/std/file2
  std::filesystem::path absolutePath = std::filesystem::absolute(
    std::filesystem::path(current_file).parent_path() / path);
  char *fileContent = Flags::readFile(absolutePath.string().c_str());
  Node::Stmt *result = parse(fileContent, absolutePath);
  if (result == nullptr) {
    ErrorClass::error(line, column, "Could not parse the imported file '" + path + "'",
                      "", "Parser Error", path.c_str(), lexer, psr->tks, true,
                      false, false, false, false, false);
    return nullptr;
  }
  // Typecheck the imported file
  TypeChecker::performCheck(result, false);

  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an import stmt");

  node.current_file = current_file;

  return new ImportStmt(line, column, path, result, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::linkStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  // @link "path";
  psr->expect(psr, TokenKind::LINK,
              "Expected a LINK keyword to start a link stmt");
  std::string path = psr->expect(psr, TokenKind::STRING,
                                 "Expected a STRING as a path in a link stmt")
                         .value;
  path.erase(path.begin()); // Erase initial "
  path.erase(path.end() - 1); // Erase final "
  psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a link stmt");
  return new LinkStmt(line, column, path, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::externStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;

  std::vector<std::string> externs;

  // @extern <"", "", "">; or @extern "C";
  psr->expect(psr, TokenKind::EXTERN,
              "Expected an EXTERN keyword to start an extern stmt");
  if (psr->current(psr).kind == TokenKind::LESS) {
    psr->expect(psr, TokenKind::LESS,
                "Expected a LESS to start an extern stmt");
    while (psr->current(psr).kind != TokenKind::GREATER) {
      std::string path = psr->expect(psr, TokenKind::STRING,
                              "Expected a STRING as a path in an extern stmt")
                      .value;
      path.erase(path.begin()); // Erase initial "
      path.erase(path.end() - 1); // Erase final "
      externs.push_back(path); 
      if (psr->current(psr).kind == TokenKind::COMMA)
        psr->expect(psr, TokenKind::COMMA,
                    "Expected a COMMA after a path in an extern stmt");
    }
    psr->expect(psr, TokenKind::GREATER,
                "Expected a GREATER to end an extern stmt");
    psr->expect(psr, TokenKind::SEMICOLON,
                "Expected a SEMICOLON at the end of an extern stmt");
    return new ExternStmt(line, column, "", externs, codegen::getFileID(psr->current_file));
  }
  std::string path = psr->expect(psr, TokenKind::STRING,
                                 "Expected a STRING as a path in an extern stmt")
                         .value;
  path.erase(path.begin()); // Erase initial "
  path.erase(path.end() - 1); // Erase final "
  psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of an extern stmt");
  return new ExternStmt(line, column, path, externs, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::breakStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(psr, TokenKind::BREAK,
              "Expected a BREAK keyword to start a break stmt");
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a break stmt");
  return new BreakStmt(line, column, codegen::getFileID(psr->current_file));
}

Node::Stmt *Parser::continueStmt(PStruct *psr, std::string name) {
  int line = psr->tks[psr->pos].line;
  int column = psr->tks[psr->pos].column;
  psr->expect(psr, TokenKind::CONTINUE,
              "Expected a CONTINUE keyword to start a continue stmt");
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a continue stmt");
  return new ContinueStmt(line, column, codegen::getFileID(psr->current_file));
}