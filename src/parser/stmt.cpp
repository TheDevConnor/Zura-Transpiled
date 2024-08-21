#include "../ast/stmt.hpp"
#include "../ast/ast.hpp"
#include "../helper/flags.hpp"
#include "parser.hpp"

Node::Stmt *Parser::parseStmt(PStruct *psr, std::string name) {
  auto stmt_it = stmt(psr, name);

  if (stmt_it != nullptr)
    return stmt_it;

  return exprStmt(psr);
}

Node::Stmt *Parser::exprStmt(PStruct *psr) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto *expr = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a expr stmt!");
  return new ExprStmt(line, column, expr);
}

Node::Stmt *Parser::blockStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a block stmt");
  std::vector<Node::Stmt *> stmts;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
    stmts.push_back(parseStmt(psr, name));
  }

  psr->expect(psr, TokenKind::RIGHT_BRACE,
              "Expected a R_BRACE to end a block stmt");
  return new BlockStmt(line, column, stmts);
}

Node::Stmt *Parser::varStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  auto isConst = psr->current(psr).kind == TokenKind::_CONST;
  psr->expect(psr, TokenKind::VAR,
              "Expected a VAR or CONST keyword to start a var stmt");

  name = psr->expect(psr, TokenKind::IDENTIFIER,
                     "Expected an IDENTIFIER after a VAR or CONST keyword")
             .value;

  psr->expect(psr, TokenKind::COLON,
              "Expected a COLON after the variable name in a var stmt to "
              "define the type of the variable");
  auto varType = parseType(psr, BindingPower::defaultValue);

  if (psr->current(psr).kind == TokenKind::SEMICOLON) {
    psr->expect(psr, TokenKind::SEMICOLON,
                "Expected a SEMICOLON at the end of a var stmt");
    return new VarStmt(line, column, isConst, name, varType, nullptr);
  }

  psr->expect(psr, TokenKind::EQUAL,
              "Expected a EQUAL after the type of the variable in a var stmt");
  auto assignedValue = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a var stmt");

  return new VarStmt(line, column, isConst, name, varType,
                     new ExprStmt(line, column, assignedValue));
}

// TODO: Implement the print stmt
// dis("print %d", .{1});
Node::Stmt *Parser::printStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

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

  return new PrintStmt(line, column, args);
}

Node::Stmt *Parser::constStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::_CONST,
              "Expected a CONST keyword to start a const stmt");
  name = psr->expect(psr, TokenKind::IDENTIFIER,
                     "Expected an IDENTIFIER after a CONST keyword")
             .value;

  psr->expect(psr, TokenKind::WALRUS,
              "Expected a WALRUS after the variable name in a const stmt");

  auto value = parseStmt(psr, name);

  return new ConstStmt(line, column, name, value);
}

Node::Stmt *Parser::funStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::FUN,
              "Expected a FUN keyword to start a function stmt");

  psr->expect(psr, TokenKind::LEFT_PAREN,
              "Expected a L_PAREN to start a function stmt");
  std::vector<std::pair<std::string, Node::Type *>> params;
  while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
    auto paramName =
        psr->expect(
               psr, TokenKind::IDENTIFIER,
               "Expected an IDENTIFIER as a parameter name in a function stmt")
            .value;
    psr->expect(psr, TokenKind::COLON,
                "Expected a COLON after the parameter name in a function stmt");
    auto paramType = parseType(psr, BindingPower::defaultValue);
    params.push_back({paramName, paramType});

    if (psr->current(psr).kind == TokenKind::COMMA)
      psr->expect(psr, TokenKind::COMMA,
                  "Expected a COMMA after a parameter in a function stmt");
  }
  psr->expect(psr, TokenKind::RIGHT_PAREN,
              "Expected a R_PAREN to end the parameters in a function stmt");

  Node::Type *returnType = parseType(psr, BindingPower::defaultValue);

  auto body = parseStmt(psr, name);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a function stmt");

  if (name == "main")
    return new fnStmt(line, column, name, params, returnType, body, true, true);
  return new fnStmt(line, column, name, params, returnType, body, false, false);
}

Node::Stmt *Parser::returnStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::RETURN,
              "Expected a RETURN keyword to start a return stmt");

  // if return
  if (psr->current(psr).kind == TokenKind::IF) {
    auto if_return = ifStmt(psr, name);
    psr->expect(psr, TokenKind::SEMICOLON,
                "Expected a SEMICOLON at the end of a return stmt");
    return new ReturnStmt(line, column, nullptr, if_return);
  }

  auto expr = parseExpr(psr, BindingPower::defaultValue);
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of a return stmt");
  return new ReturnStmt(line, column, expr);
}

Node::Stmt *Parser::ifStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::IF, "Expected an IF keyword to start an if stmt");
  auto condition = parseExpr(psr, BindingPower::defaultValue);

  auto thenStmt = parseStmt(psr, name);
  Node::Stmt *elseStmt = nullptr;

  if (psr->current(psr).kind == TokenKind::ELSE) {
    psr->expect(psr, TokenKind::ELSE,
                "Expected an ELSE keyword to start the else stmt");
    elseStmt = parseStmt(psr, name);
  }

  return new IfStmt(line, column, condition, thenStmt, elseStmt);
}

Node::Stmt *Parser::structStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::STRUCT,
              "Expected a STRUCT keyword to start a struct stmt");

  psr->expect(psr, TokenKind::LEFT_BRACE,
              "Expected a L_BRACE to start a struct stmt");

  std::vector<std::pair<std::string, Node::Type *>> fields;
  std::vector<Node::Stmt *> stmts;

  while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
    auto tokenKind = psr->current(psr).kind;
    switch (tokenKind) {
    case TokenKind::IDENTIFIER: {
      auto fieldName =
          psr->expect(psr, TokenKind::IDENTIFIER,
                      "Expected an IDENTIFIER as a field name in a struct stmt")
              .value;
      psr->expect(psr, TokenKind::COLON,
                  "Expected a COLON after the field name in a struct stmt");
      auto fieldType = parseType(psr, BindingPower::defaultValue);
      fields.push_back({fieldName, fieldType});
      psr->expect(psr, TokenKind::SEMICOLON,
                  "Expected a SEMICOLON after the field type in a struct stmt");
      break;
    }
    case TokenKind::SEMICOLON:
      psr->expect(psr, TokenKind::SEMICOLON,
                  "Expected a SEMICOLON after a field in a struct stmt");
      break;
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
    return new StructStmt(line, column, name, fields, stmts);
  return new StructStmt(line, column, name, fields, {});
}

Node::Stmt *Parser::loopStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

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

    // we need to look two tokens ahead to determine if it is a for loop or a while loop
    if (psr->peek(psr, 1).kind == TokenKind::EQUAL) {
      isForLoop = true;
      forLoop = parseExpr(psr, BindingPower::defaultValue);
      
      psr->expect(psr, TokenKind::SEMICOLON,
                  "Expected a SEMICOLON after the for loop condition in a loop stmt");

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

    auto body = parseStmt(psr, name);

    if (isOptional) {
      if (isForLoop)
        return new ForStmt(line, column, forLoop, condition, opCondition, body);
      return new WhileStmt(line, column, whileLoop, opCondition, body);
    }
    if (isForLoop)
      return new ForStmt(line, column, forLoop, condition, nullptr, body);
    return new WhileStmt(line, column, whileLoop, nullptr, body);
  }

  return nullptr;
}

Node::Stmt *Parser::enumStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

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
    psr->expect(psr, TokenKind::COMMA,
                "Expected a COMMA after a field in an enum stmt");
  }
  psr->expect(psr, TokenKind::RIGHT_BRACE,
              "Expected a R_BRACE to end an enum stmt");
  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an enum stmt");

  return new EnumStmt(line, column, name, fields);
}

Node::Stmt *Parser::templateStmt(PStruct *psr, std::string name) {
  // template <typealias T>
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

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

  return new TemplateStmt(typeParams, line, column);
}

Node::Stmt *Parser::importStmt(PStruct *psr, std::string name) {
  auto line = psr->tks[psr->pos].line;
  auto column = psr->tks[psr->pos].column;

  psr->expect(psr, TokenKind::IMPORT,
              "Expected an IMPORT keyword to start an import stmt");

  auto path = psr->expect(psr, TokenKind::STRING,
                          "Expected a STRING as a path in an import stmt")
                  .value;
  path = path.substr(1, path.size() - 2); // removes "" from the path
  auto result = parse(Flags::readFile(path.c_str()), path);
  if (result == nullptr) {
    ErrorClass::error(line, column, "Could not parse the file '" + path + "'",
                      "", "Parser Error", path.c_str(), lexer, psr->tks, true,
                      false, false, false, false);
    return nullptr;
  }

  psr->expect(psr, TokenKind::SEMICOLON,
              "Expected a SEMICOLON at the end of an import stmt");

  return new ImportStmt(line, column, path, result);
}