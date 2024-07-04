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
    psr->expect(psr, TokenKind::SEMICOLON);
    return new ExprStmt(expr);
}

Node::Stmt *Parser::blockStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::LEFT_BRACE);
    std::vector<Node::Stmt *> stmts;

    while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) {
        stmts.push_back(parseStmt(psr, name));
    }

    psr->expect(psr, TokenKind::RIGHT_BRACE);
    return new BlockStmt(stmts);
}

Node::Stmt *Parser::varStmt(PStruct *psr, std::string name) { 
    auto isConst = psr->current(psr).kind == TokenKind::CONST;
    if (isConst) psr->expect(psr, TokenKind::CONST); else psr->expect(psr, TokenKind::VAR);

    name = psr->expect(psr, TokenKind::IDENTIFIER).value;

    psr->expect(psr, TokenKind::COLON);
    auto varType = parseType(psr, BindingPower::defaultValue);

    if (psr->current(psr).kind == TokenKind::SEMICOLON) {
        psr->expect(psr, TokenKind::SEMICOLON);
        return new VarStmt(isConst, name, varType, nullptr);    
    }

    psr->expect(psr, TokenKind::EQUAL);
    auto assignedValue = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON);

    return new VarStmt(isConst, name, varType, new ExprStmt(assignedValue));
}

Node::Stmt *Parser::constStmt(PStruct *psr, std::string name) {
   psr->expect(psr, TokenKind::CONST);
   name = psr->expect(psr, TokenKind::IDENTIFIER).value; 

   psr->expect(psr, TokenKind::WALRUS);

   auto value = parseStmt(psr, name);

   return new ConstStmt(name, value);
}

Node::Stmt *Parser::funStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::FUN);

    psr->expect(psr, TokenKind::LEFT_PAREN);
    std::vector<std::pair<std::string, Node::Type *>> params;
    while (psr->current(psr).kind != TokenKind::RIGHT_PAREN) {
        auto paramName = psr->expect(psr, TokenKind::IDENTIFIER).value;
        psr->expect(psr, TokenKind::COLON);
        auto paramType = parseType(psr, BindingPower::defaultValue);
        params.push_back({paramName, paramType});

        if (psr->current(psr).kind == TokenKind::COMMA) psr->expect(psr, TokenKind::COMMA); 
    }
    psr->expect(psr, TokenKind::RIGHT_PAREN);

    Node::Type *returnType = parseType(psr, BindingPower::defaultValue);

    auto body = parseStmt(psr, name);
    psr->expect(psr, TokenKind::SEMICOLON);
    
    return new fnStmt(name, params, returnType, body);
}

Node::Stmt *Parser::returnStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::RETURN);
    auto expr = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::SEMICOLON);
    return new ReturnStmt(expr);
}

Node::Stmt *Parser::ifStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::IF);
    psr->expect(psr, TokenKind::LEFT_PAREN);
    auto condition = parseExpr(psr, BindingPower::defaultValue);
    psr->expect(psr, TokenKind::RIGHT_PAREN);

    auto thenStmt = parseStmt(psr, name);
    Node::Stmt *elseStmt = nullptr;

    if (psr->current(psr).kind == TokenKind::ELSE) {
        psr->expect(psr, TokenKind::ELSE);
        elseStmt = parseStmt(psr, name);
    }

    return new IfStmt(condition, thenStmt, elseStmt);
}

Node::Stmt *Parser::structStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::STRUCT);

    psr->expect(psr, TokenKind::LEFT_BRACE);

    std::vector<std::pair<std::string, Node::Type *>> fields;
    std::vector<Node::Stmt *> stmts;

    while (psr->current(psr).kind != TokenKind::RIGHT_BRACE) { 
        auto fieldName = psr->expect(psr, TokenKind::IDENTIFIER).value;
        psr->expect(psr, TokenKind::COLON);
        auto fieldType = parseType(psr, BindingPower::defaultValue);
        fields.push_back({fieldName, fieldType});
        psr->expect(psr, TokenKind::SEMICOLON); 

        if (psr->current(psr).kind == TokenKind::CONST) {
            auto constStmt = parseStmt(psr, name);
            stmts.push_back(constStmt);
        }
    }

    psr->expect(psr, TokenKind::RIGHT_BRACE);
    psr->expect(psr, TokenKind::SEMICOLON);

    if (stmts.size() > 0)
        return new StructStmt(name, fields, stmts);
    return new StructStmt(name, fields, {});
}

Node::Stmt *Parser::loopStmt(PStruct *psr, std::string name) {
    psr->expect(psr, TokenKind::LOOP);

    psr->expect(psr, TokenKind::LEFT_PAREN);

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
            varName = psr->expect(psr, TokenKind::IDENTIFIER).value;
            psr->expect(psr, TokenKind::IN);
            forLoop = parseExpr(psr, BindingPower::defaultValue);
        } else {
            whileLoop = parseExpr(psr, BindingPower::defaultValue);
        }
    }
    psr->expect(psr, TokenKind::RIGHT_PAREN);

    if (psr->current(psr).kind == TokenKind::COLON) {
        isOptional = true;
        psr->expect(psr, TokenKind::COLON);
        psr->expect(psr, TokenKind::LEFT_PAREN);
        opCondition = parseExpr(psr, BindingPower::defaultValue);
        psr->expect(psr, TokenKind::RIGHT_PAREN);
    }

    auto body = parseStmt(psr, name);

    if(isOptional) {
        if(isForLoop) return new ForStmt(varName, forLoop, opCondition, body); 
        return new WhileStmt(whileLoop, opCondition, body);
    }
    if(isForLoop) return new ForStmt(varName, forLoop, nullptr, body);
    return new WhileStmt(whileLoop, nullptr, body);
}