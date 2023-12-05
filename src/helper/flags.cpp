#include <iostream>
#include <fstream>

#include "../../inc/colorize.hpp"
#include "../parser/parser.hpp"
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"
#include "../common.hpp"
#include "flags.hpp"

using namespace std;

void Flags::compilerDelete(char** argv) {
    cout << "Deleting the executable file" << endl;
        std::string outName = argv[1];
        char rmCommand[256];

        #ifdef _WIN32
            strcat(rmCommand, "del ");
            strcat(rmCommand, outName.c_str());
            strcat(rmCommand, ".exe");
            system(rmCommand);
        #else
            strcat(rmCommand, "rm -rf ");
            strcat(rmCommand, outName.c_str());
            system(rmCommand);
        #endif
        Exit(ExitValue::FLAGS_PRINTED);
}

void Flags::compileToC(std::string name) {
    std::string compileCommand = "gcc -o " + name + " out.c";
    system(compileCommand.c_str());
}

char* Flags::readFile(const char* path) {
    ifstream file(path, ios::binary);
    if (!file) {
        cerr << "Error: Could not open file '" << path << "'" << endl;
        Exit(ExitValue::INVALID_FILE);
    }

    file.seekg(0, ios::end);
    size_t size = file.tellg();
    file.seekg(0, ios::beg);

    char* buffer = new char[size + 1];
    file.read(buffer, size);
    file.close();

    buffer[size] = 0;
    return buffer;
}

void Flags::runFile(const char* path, std::string outName, bool save) {
    const char* source = readFile(path);

    // Lexer lexer(source);
    // Lexer::Token token = lexer.scanToken();

    // while (token.kind != TokenKind::END_OF_FILE) {
    //     cout << "Token: " << token.kind << endl;
    //     token = lexer.scanToken();
    // }

    // Parser parser(token, false);

    // Ast node test
    AstNode* literal5 = new AstNode(AstNodeType::LITERAL, new AstNode::Literal(
                        Lexer::Token({.start = "5", .kind = TokenKind::NUMBER, .current = 0, .column = 0, .line = 0})));
    AstNode* literal4 = new AstNode(AstNodeType::LITERAL, new AstNode::Literal(
                        Lexer::Token({.start = "4", .kind = TokenKind::NUMBER, .current = 0, .column = 0, .line = 0})));

    AstNode* binaryPLUS  = new AstNode(AstNodeType::BINARY, new AstNode::Binary(TokenKind::PLUS, literal5, literal4));
    AstNode* binaryMINUS = new AstNode(AstNodeType::BINARY, new AstNode::Binary(TokenKind::MINUS, binaryPLUS, literal4));
    AstNode* binary = new AstNode(AstNodeType::BINARY, new AstNode::Binary(TokenKind::PLUS, binaryMINUS, literal5));

    // The output should be:
    // (5 + 4) - 4 + 5
    // This in the ast form should be:
    // Binary: 7
    //     Binary: 6
    //         Binary: 7
    //             Literal: 5
    //             Literal: 4
    //         Literal: 4
    //     Literal: 5
    
    AstNode::printAst(binary, 0);


    // clean up
    delete literal5;
    delete literal4;
    delete binaryPLUS;
    delete binaryMINUS;
    delete binary;

    // lexer.~Lexer();
    // parser.~Parser();
    delete[] source;

    if (!save) {
        #ifdef _WIN32
            system("del out.c");
        #else
            system("rm -rf out.c");
        #endif
    }
}
