#include <iostream>
#include "ast.hpp"

void AstNode::printAst(AstNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) std::cout << " ";

    switch (node->type) {
        case AstNodeType::BINARY: {
            AstNode::Binary* binary = (AstNode::Binary*)node->data;
            std::cout << "Binary: " << binary->op << std::endl;
            printAst(binary->left, indent + 2);
            printAst(binary->right, indent + 2);
            break;
        }
        case AstNodeType::GROUPING: {
            AstNode::Grouping* grouping = (AstNode::Grouping*)node->data;
            std::cout << "Grouping: " << std::endl;
            printAst(grouping->expression, indent + 2);
            break;
        }
        case AstNodeType::LITERAL: {
            AstNode::Literal* literal = (AstNode::Literal*)node->data;
            std::cout << "Literal-Kind: " << literal->literal.kind << std::endl;
            break;
        }
        case AstNodeType::UNARY: {
            AstNode::Unary* unary = (AstNode::Unary*)node->data;
            std::cout << "Unary: " << unary->op << std::endl;
            printAst(unary->right, indent + 2);
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