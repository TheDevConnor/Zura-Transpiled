#include <iostream>
#include "ast.hpp"

void AstNode::printAst(AstNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) std::cout << " ";

    switch (node->type) {
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
        
        case AstNodeType::EXPRESSION: {
            AstNode::Expression* expression = (AstNode::Expression*)node->data;
            std::cout << "Expression: " << std::endl;
            printAst(expression->expression, indent + 2);
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