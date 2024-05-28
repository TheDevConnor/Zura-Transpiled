cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

# Specify header and source files
set(ZURA_HEADER_FILES
    src/helper/error/error.hpp
    src/helper/flags.hpp
    inc/colorize.hpp

    # Lexer Files
    src/lexer/lexer.hpp
    src/lexer/maps.hpp

    # Ast Files
    src/ast/ast.hpp
    src/ast/expr.hpp
    src/ast/stmt.hpp

    # Parser Files
    src/parser/parser.hpp
    src/parser/map.hpp

    inc/update.hpp
    src/common.hpp
)

set(ZURA_SOURCE_FILES
    src/helper/error/error.cpp
    src/helper/flags.cpp
    src/lexer/lexer.cpp

    # Parser Files
    src/parser/parser.cpp
    src/parser/expr.cpp 
    
    src/main.cpp
)

# Create executable
add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})



