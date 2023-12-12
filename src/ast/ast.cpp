#include <iostream>
#include "ast.hpp"

void AstNode::printVarDeclaration(AstNode::VarDeclaration* varDeclaration, int indent) {
    for (int i = 0; i < indent; i++) std::cout << " ";

    std::cout << "Name: " << varDeclaration->name.start << std::endl;

    if (varDeclaration->type) {
        for (int i = 0; i < indent; i++) std::cout << " ";
        std::cout << "Type: " << std::endl;
        printAst(varDeclaration->type, indent + 2);
    }

    if (varDeclaration->initializer) {
        for (int i = 0; i < indent; i++) std::cout << " ";
        std::cout << "Initializer: " << std::endl;
        printAst(varDeclaration->initializer, indent + 2);
    }
}

void AstNode::printAst(AstNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) std::cout << " ";

    switch (node->type) {
        case AstNodeType::PROGRAM: {
            AstNode::Program* program = (AstNode::Program*)node->data;
            std::cout << "Program:" << std::endl;
            for (AstNode* stmt : program->statements) printAst(stmt, 1);
            break;
        }

        case AstNodeType::BINARY: {
            AstNode::Binary* binary = (AstNode::Binary*)node->data;
            std::cout << "Binary-TK-Kind: " << binary->op << std::endl;
            printAst(binary->left, indent + 3);
            printAst(binary->right, indent + 3);
            break;
        }
        case AstNodeType::UNARY: {
            AstNode::Unary* unary = (AstNode::Unary*)node->data;
            std::cout << "Unary-TK-Kind: " << unary->op << std::endl;
            printAst(unary->right, indent + 2);
            break;
        }
        case AstNodeType::GROUPING: {
            AstNode::Grouping* grouping = (AstNode::Grouping*)node->data;
            std::cout << "Grouping: " << std::endl;
            printAst(grouping->expression, indent + 2);
            break;
        }

        case AstNodeType::NUMBER_LITERAL: {
            AstNode::NumberLiteral* numberLiteral = (AstNode::NumberLiteral*)node->data;
            std::cout << "NumberLiteral: " << numberLiteral->value << std::endl;
            break;
        }
        case AstNodeType::STRING_LITERAL: {
            AstNode::StringLiteral* stringLiteral = (AstNode::StringLiteral*)node->data;
            std::cout << "StringLiteral: " << stringLiteral->value << std::endl;
            break;
        }
        case AstNodeType::TRUE_LITERAL: {
            std::cout << "TrueLiteral" << std::endl;
            break;
        }
        case AstNodeType::FALSE_LITERAL: {
            std::cout << "FalseLiteral" << std::endl;
            break;
        }
        case AstNodeType::NIL_LITERAL: {
            std::cout << "NilLiteral" << std::endl;
            break;
        }

        case AstNodeType::TYPE: {
            AstNode::Type* type = (AstNode::Type*)node->data;
            std::cout << "Type-TK-Kind: " << type->type.kind << std::endl;
            break;
        }
        
        case AstNodeType::EXPRESSION: {
            AstNode::Expression* expression = (AstNode::Expression*)node->data;
            std::cout << "Expression: " << std::endl;
            printAst(expression->expression, indent + 2);
            break;
        }
        case AstNodeType::VAR_DECLARATION: {
            AstNode::VarDeclaration* varDeclaration = (AstNode::VarDeclaration*)node->data;
            std::cout << "VarDeclaration: " << std::endl;
            printVarDeclaration(varDeclaration, indent + 2);
            break;
        }
        case AstNodeType::PRINT: {
            AstNode::Print* print = (AstNode::Print*)node->data;
            std::cout << "Print: " << std::endl;
            printAst(print->expression, indent + 2);
            break;
        }

        default:
            std::cout << "Unknown node type" << std::endl;
            break;
    }
}