cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

# Specify header and source files
set(ZURA_HEADER_FILES
    src/helper/error/error.hpp
    src/helper/flags.hpp
    src/helper/color.hpp

    # Lexer Files
    src/lexer/lexer.hpp

    # Ast Files
    src/ast/ast.hpp
    src/ast/expr.hpp
    src/ast/stmt.hpp
    src/ast/types.hpp

    # Parser Files
    src/parser/parser.hpp

    # Type Checker Files
    src/typeChecker/type.hpp

    inc/update.hpp
    src/common.hpp
)

set(ZURA_SOURCE_FILES
    src/helper/error/error.cpp
    src/helper/flags.cpp
    src/lexer/lexer.cpp
    src/lexer/maps.cpp

    # Parser Files
    src/parser/parser.cpp
    src/parser/expr.cpp
    src/parser/stmt.cpp
    src/parser/type.cpp
    src/parser/tmaps.cpp
    src/parser/map.cpp 

    # Type Checker Files
    src/typeChecker/expr.cpp
    src/typeChecker/stmt.cpp
    src/typeChecker/type.cpp
    src/typeChecker/map.cpp
    
    src/main.cpp
)

# Create executable
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})
add_link_options(-lstdc++)