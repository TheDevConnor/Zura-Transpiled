// #include <cstring>
// #include <iostream>

// #include "../helper/error/error.hpp"
// #include "../ast/ast.hpp"
// #include "type.hpp"

// void Type::checkExpression(AstNode *expr) {
//   if (expr->type == AstNodeType::NUMBER_LITERAL) {
//     AstNode::NumberLiteral *numberLiteral =
//         (AstNode::NumberLiteral *)expr->data;
//     determineType(numberLiteral->value);
//   }
//   if (expr->type == AstNodeType::STRING_LITERAL) {
//     returnType = findType["str"];
//   }
//   if (expr->type == AstNodeType::IDENTIFIER) {
//     AstNode::Identifier *identifier = (AstNode::Identifier *)expr->data;
//     identifier->name.start = strtok(const_cast<char *>(identifier->name.start), ";");

//     for (auto& param : paramData) {
//       if (param.first == identifier->name.start) {
//         returnType = param.second;
//         return;
//       }
//     }
//   }
//   if (expr->type == AstNodeType::BINARY) {
//     AstNode::Binary *binary = (AstNode::Binary *)expr->data;

//     checkExpression(binary->left);
//     checkExpression(binary->right);

//     if (binary->left->type == AstNodeType::NUMBER_LITERAL &&
//         binary->right->type == AstNodeType::NUMBER_LITERAL) {
//       AstNode::NumberLiteral *left =
//           (AstNode::NumberLiteral *)binary->left->data;
//       AstNode::NumberLiteral *right =
//           (AstNode::NumberLiteral *)binary->right->data;

//       switch (binary->op) {
//         case TokenKind::PLUS:
//           determineType(left->value + right->value);
//           break;
//         case TokenKind::MINUS:
//           determineType(left->value - right->value);
//           break;
//         case TokenKind::STAR:
//           determineType(left->value * right->value);
//           break;
//         case TokenKind::SLASH:
//           determineType(left->value / right->value);
//           break;
//         default:
//           break;
//       }
//     }
//   }
// }

// void Type::checkBody(AstNode *body) {
//   AstNode::Block *block = (AstNode::Block *)body->data;

//   if (block->statements.size() > 0) {
//     AstNode *stmt = block->statements[block->statements.size() - 1];
//     switch (stmt->type) {
//       case AstNodeType::EXIT: {
//         AstNode::Exit *exit = (AstNode::Exit *)stmt->data;
//         checkExpression(exit->expression);
//         break;
//       }
//       case AstNodeType::RETURN: {
//         AstNode::Return *return_ = (AstNode::Return *)stmt->data;
//         checkExpression(return_->expression);
//         break;
//       }
//       default:
//         checkBody(block->statements[block->statements.size() - 1]);
//         break;
//     }
//   }
// }

// void Type::typeCheck(AstNode *expression) {
//   if (expression->type == AstNodeType::PROGRAM) {
//     AstNode::Program *program = (AstNode::Program *)expression->data;

//     for (AstNode *stmt : program->statements) {
//       switch (stmt->type) {
//       case AstNodeType::FUNCTION_DECLARATION: {
//         AstNode::FunctionDeclaration *functionDeclaration =
//             (AstNode::FunctionDeclaration *)stmt->data;

//         functionDeclaration->name.start =
//             strtok(const_cast<char *>(functionDeclaration->name.start), "(");
//         functionDeclaration->name.start =
//             strtok(const_cast<char *>(functionDeclaration->name.start), " ");
//         name = functionDeclaration->name.start;

//         // add the param name and type to the param vector
//         if (functionDeclaration->parameters.size() > 0) {
//           for (auto& param : functionDeclaration->parameters) {
//             AstNode::Type *type_ = (AstNode::Type *)functionDeclaration->paramType.front()->data;
//             type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
//             type_->type.start = strtok(const_cast<char *>(type_->type.start), ",");
//             type_->type.start = strtok(const_cast<char *>(type_->type.start), ")");

//             param.start = strtok(const_cast<char *>(param.start), ":");

//             paramData.push_back(std::make_pair(param.start, type_));
//           }

//           for (int i = 0; i < functionDeclaration->parameters.size(); i++) {
//             functionDeclaration->paramType.erase(
//                 functionDeclaration->paramType.begin());
//           }
//         }

//         AstNode::Type *type_ = (AstNode::Type *)functionDeclaration->type->data;
//         type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
//         type = type_;

//         checkBody(functionDeclaration->body);
//         Error::errorType(type, returnType, name);
//         resetType();

//         break;
//       }
//       case AstNodeType::VAR_DECLARATION: {
//         AstNode::VarDeclaration *varDeclaration =
//             (AstNode::VarDeclaration *)stmt->data;

//         name = varDeclaration->name.start;

//         AstNode::Type *type_ = (AstNode::Type *)varDeclaration->type->data;
//         type_->type.start = strtok(const_cast<char *>(type_->type.start), " ");
//         type = type_;

//         checkExpression(varDeclaration->initializer);
//         Error::errorType(type, returnType, name);
//         resetType();
//         break;
//       }
//       default:
//         break;
//       }
//     }
//     return;
//   }
// }
