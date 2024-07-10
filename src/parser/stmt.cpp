#include "parser.hpp"
#include "../ast/ast.hpp"
#include "../ast/stmt.hpp"

Node::Stmt *Parser::parseStmt(PStruct *psr, std::string name) {
    auto stmt_it = stmt(psr, name);

    if (stmt_it != nullptr)
        return stmt_it;

    return exprStmt(psr);
}

Node::Stmt *Parser::exprStmt(PStruct *psr) {
    auto *expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a expr stmt!");
    return new ExprStmt(expr);
}

Node::Stmt *Parser::blockStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::LEFT_BRACE, "Expected a L_BRACE to start a block stmt");
    std::vector<Node::Stmt *> stmts;

    while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
        stmts.push_back(parseStmt(psr, name));
    }

    psr->expect(psr, TokenKind::RIGHT_BRACE, "Expected a R_BRACE to end a block stmt");
    return new BlockStmt(stmts);
}

Node::Stmt *Parser::varStmt(PStruct *psr, std::string name) { 
    auto isConst = psr->current(psr).kind == TokenKind::CONST;
    psr->expect(psr, TokenKind::VAR, "Expected a VAR or CONST keyword to start a var stmt");

    name = psr->expect(psr, TokenKind::IDENTIFIER, "Expected an IDENTIFIER after a VAR or CONST keyword").value;

    psr->expect(psr, TokenKind::COLON, "Expected a COLON after the variable name in a var stmt to define the type of the variable");
    auto varType = parseType(psr, BindingPower::defaultValue);

    if (psr->current(psr).kind == TokenKind::SEMICOLON) {
        psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a var stmt");
        return new VarStmt(isConst, name, varType, nullptr);    
    }

    psr->expect(psr, TokenKind::EQUAL, "Expected a EQUAL after the type of the variable in a var stmt");
    auto assignedValue = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a var stmt");

    return new VarStmt(isConst, name, varType, new ExprStmt(assignedValue));
}

// dis("print %d", .{1});
Node::Stmt *Parser::printStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::PRINT, "Expected a PRINT keyword to start a print stmt");
    psr->expect(psr, TokenKind::LEFT_PAREN, "Expected a L_PAREN to start a print stmt");

    std::vector<Node::Expr *> args; // Change the type of args vector

    while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
        args.push_back(parseExpr(psr, BindingPower::defaultValue));
    }

    psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end a print stmt");
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a print stmt");

    return new PrintStmt(args);
}

Node::Stmt *Parser::constStmt(PStruct *psr, std::string name) {
   psr->expect(psr, TokenKind::CONST, "Expected a CONST keyword to start a const stmt");
   name = psr->expect(psr, TokenKind::IDENTIFIER, "Expected an IDENTIFIER after a CONST keyword").value;

   psr->expect(psr, TokenKind::WALRUS, "Expected a WALRUS after the variable name in a const stmt");

   auto value = parseStmt(psr, name);

   return new ConstStmt(name, value);
}

Node::Stmt *Parser::funStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::FUN, "Expected a FUN keyword to start a function stmt");

    psr->expect(psr, TokenKind::LEFT_PAREN, "Expected a L_PAREN to start a function stmt");
    std::vector<std::pair<std::string, Node::Type *>> params;
    while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
        auto paramName = psr->expect(psr, TokenKind::IDENTIFIER, "Expected an IDENTIFIER as a parameter name in a function stmt").value;
        psr->expect(psr, TokenKind::COLON, "Expected a COLON after the parameter name in a function stmt");
        auto paramType = parseType(psr, BindingPower::defaultValue);
        params.push_back({paramName, paramType});

        if (psr->current(psr).kind == TokenKind::COMMA) 
            psr->expect(psr, TokenKind::COMMA, "Expected a COMMA after a parameter in a function stmt"); 
    }
    psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end the parameters in a function stmt");

    Node::Type *returnType = parseType(psr, BindingPower::defaultValue);

    auto body = parseStmt(psr, name);
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a function stmt");
    
    return new fnStmt(name, params, returnType, body);
}

Node::Stmt *Parser::returnStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::RETURN, "Expected a RETURN keyword to start a return stmt");
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a return stmt");
    return new ReturnStmt(expr);
}

Node::Stmt *Parser::ifStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::IF, "Expected an IF keyword to start an if stmt");
    psr->expect(psr, TokenKind::LEFT_PAREN, "Expected a L_PAREN to start an if stmt");
    auto condition = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end the condition in an if stmt");

    auto thenStmt = parseStmt(psr, name);
    Node::Stmt *elseStmt = nullptr;

    if (psr->current(psr).kind == TokenKind::ELSE) {
        psr->expect(psr, TokenKind::ELSE, "Expected an ELSE keyword to start the else stmt");
        elseStmt = parseStmt(psr, name);
    }

    return new IfStmt(condition, thenStmt, elseStmt);
}

Node::Stmt *Parser::structStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::STRUCT, "Expected a STRUCT keyword to start a struct stmt");

    psr->expect(psr, TokenKind::LEFT_BRACE, "Expected a L_BRACE to start a struct stmt");

    std::vector<std::pair<std::string, Node::Type *>> fields;
    std::vector<Node::Stmt *> stmts;

    while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
        auto tokenKind = psr->current(psr).kind;
        switch (tokenKind) {
            case TokenKind::IDENTIFIER: {
                auto fieldName = psr->expect(psr, TokenKind::IDENTIFIER, "Expected an IDENTIFIER as a field name in a struct stmt").value;
                psr->expect(psr, TokenKind::COLON, "Expected a COLON after the field name in a struct stmt");
                auto fieldType = parseType(psr, BindingPower::defaultValue);
                fields.push_back({fieldName, fieldType});
                psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON after the field type in a struct stmt");
                break;
            }
            case TokenKind::SEMICOLON:
                psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON after a field in a struct stmt");
                break;
            case TokenKind::CONST:
                stmts.push_back(constStmt(psr, name));
                break;
            default:
                stmts.push_back(parseStmt(psr, name));
                break;
        }
    }

    psr->expect(psr, TokenKind::RIGHT_BRACE, "Expected a R_BRACE to end a struct stmt");
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of a struct stmt");

    if (stmts.size() > 0)
        return new StructStmt(name, fields, stmts);
    return new StructStmt(name, fields, {});
}

Node::Stmt *Parser::loopStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::LOOP, "Expected a LOOP keyword to start a loop stmt");

    psr->expect(psr, TokenKind::LEFT_PAREN, "Expected a L_PAREN to start a loop stmt");

    std::string varName;
    Node::Expr *forLoop;
    Node::Expr *whileLoop;
    Node::Expr *opCondition;
    bool isForLoop = false;
    bool isOptional = false;

    while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
        // First condition is the for loop condition
        // Second condition is the while loop condition 
        if (psr->peek(psr, 1).kind == TokenKind::IN) {
            isForLoop = true;
            varName = psr->expect(psr, TokenKind::IDENTIFIER, "Expected an IDENTIFIER as a variable name in a for loop stmt").value;
            psr->expect(psr, TokenKind::IN, "Expected an IN keyword to start a for loop stmt");
            forLoop = parseExpr(psr, BindingPower::defaultValue);
        } else {
            whileLoop = parseExpr(psr, BindingPower::defaultValue);
        }
    }
    psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end the condition in a loop stmt");

    if (psr->current(psr).kind == TokenKind::COLON) {
        isOptional = true;
        psr->expect(psr, TokenKind::COLON, "Expected a COLON to start the body of a loop stmt");
        psr->expect(psr, TokenKind::LEFT_PAREN, "Expected a L_PAREN to start the condition in a loop stmt");
        opCondition = parseExpr(psr, BindingPower::defaultValue);
        psr->expect(psr, TokenKind::RIGHT_PAREN, "Expected a R_PAREN to end the condition in a loop stmt");
    }

    auto body = parseStmt(psr, name);

    if(isOptional) {
        if(isForLoop) return new ForStmt(varName, forLoop, opCondition, body); 
        return new WhileStmt(whileLoop, opCondition, body);
    }
    if(isForLoop) return new ForStmt(varName, forLoop, nullptr, body);
    return new WhileStmt(whileLoop, nullptr, body);
}

Node::Stmt *Parser::enumStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::ENUM, "Expected an ENUM keyword to start an enum stmt");
    psr->expect(psr, TokenKind::LEFT_BRACE, "Expected a L_BRACE to start an enum stmt");

    std::vector<std::string> fields;
    while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
        fields.push_back(psr->expect(psr, TokenKind::IDENTIFIER, "Expected an IDENTIFIER as a field name in an enum stmt").value);
        if (psr->current(psr).kind == TokenKind::COMMA) 
            psr->expect(psr, TokenKind::COMMA, "Expected a COMMA after a field in an enum stmt");
    }
    psr->expect(psr, TokenKind::RIGHT_BRACE, "Expected a R_BRACE to end an enum stmt");
    psr->expect(psr, TokenKind::SEMICOLON, "Expected a SEMICOLON at the end of an enum stmt");

    return new EnumStmt(name, fields);
}