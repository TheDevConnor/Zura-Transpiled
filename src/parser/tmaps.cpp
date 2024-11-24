#include "../ast/types.hpp"
#include "parser.hpp"

/**
 * @brief Looks up a value in a vector of key-value pairs.
 *
 * This function searches for a key in the given vector of key-value pairs and
 * returns the corresponding value. If the key is not found, an error is
 * generated.
 *
 * @tparam T The type of the value in the key-value pairs.
 * @tparam U The type of the key in the key-value pairs.
 * @param psr A pointer to the PStruct object.
 * @param lu The vector of key-value pairs to search in.
 * @param key The key to search for.
 * @return The value corresponding to the key.
 */
template <typename T, typename U>
T Parser::lookup(PStruct *psr, const std::vector<std::pair<U, T>> &lu, U key) {
  std::vector<std::pair<U, T>>::iterator it = std::find_if(lu.begin(), lu.end(),
                         [key](std::pair<U, T> &p) { return p.first == key; });

  if (it == lu.end()) {
    ErrorClass::error(0, 0, "No value found for key in Type maps", "",
                      "Parser Error", node.current_file, lexer, psr->tks, true,
                      false, false, false, false, false);
  }

  return it->second;
}

/**
 * Creates the type maps for the Parser.
 *
 * This function initializes the type_nud_lu, type_led_lu, and type_bp_lu maps
 * used by the Parser class for parsing types. The type_nud_lu map associates
 * token kinds with corresponding parsing functions for prefix expressions,
 * while the type_led_lu map associates token kinds with parsing functions for
 * infix expressions. The type_bp_lu map is used to determine the binding power
 * of infix operators.
 */
void Parser::createTypeMaps() {
  type_nud_lu = {
      {TokenKind::IDENTIFIER, symbol_table},
      {TokenKind::LEFT_BRACKET, array_type},
      {TokenKind::STAR, pointer_type}, // *
      {TokenKind::LAND, pointer_type}, // &
  };
  type_led_lu = {};
  type_bp_lu = bp_lu;
}

/**
 * This function is responsible for handling the type-led expressions in the
 * parser. It takes a PStruct pointer, a left Node::Type pointer, and a
 * BindingPower value as parameters. It looks up the appropriate function to
 * handle the type-led expression based on the operator kind. If an exception
 * occurs during the lookup or evaluation, an error message is generated.
 *
 * @param psr The PStruct pointer representing the parser.
 * @param left The left Node::Type pointer.
 * @param bp The BindingPower value.
 * @return The resulting Node::Type pointer after evaluating the type-led
 * expression.
 */
Node::Type *Parser::type_led(PStruct *psr, Node::Type *left, BindingPower bp) {
  Lexer::Token op = psr->current(psr);
  try {
    return lookup(psr, type_led_lu, op.kind)(psr, left, bp);
  } catch (std::exception &e) {
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Error in type_led: " + std::string(e.what()), "",
                      "Parser Error", node.current_file, lexer, psr->tks, true,
                      false, false, false, false, false);
    return nullptr;
  }
  return nullptr;
}

/**
 * This function is responsible for handling the type-nud expressions in the
 * parser. It takes a PStruct pointer as a parameter and looks up the
 * appropriate function to handle the type-nud expression based on the operator
 * kind. If an exception occurs during the lookup or evaluation, an error
 * message is generated.
 *
 * @param psr The PStruct pointer representing the parser.
 * @return The resulting Node::Type pointer after evaluating the type-nud
 * expression.
 */
Node::Type *Parser::type_nud(PStruct *psr) {
  Lexer::Token op = psr->current(psr);
  try {
    return Parser::lookup(psr, type_nud_lu, op.kind)(psr);
  } catch (std::exception &e) {
    ErrorClass::error(psr->current(psr).line, psr->current(psr).column,
                      "Error in type_nud: " + std::string(e.what()), "",
                      "Parser Error", psr->current_file, lexer, psr->tks, true,
                      false, false, false, false, false);
    return nullptr;
  }
  return nullptr;
}

/**
 * This function is responsible for retrieving the binding power of a given
 * token kind. It takes a PStruct pointer and a TokenKind value as parameters
 * and returns the corresponding binding power value from the type_bp_lu map.
 *
 * @param psr The PStruct pointer representing the parser.
 * @param tk The TokenKind value to retrieve the binding power for.
 * @return The binding power value for the given token kind.
 */
Parser::BindingPower Parser::type_getBP(PStruct *psr, TokenKind tk) {
  return lookup(psr, type_bp_lu, tk);
}
