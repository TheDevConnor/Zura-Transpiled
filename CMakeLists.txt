cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

set(ZURA_HEADER_FILES
    src/helper/error/lexerError.hpp
    src/helper/flags.hpp
    src/lexer/lexer.hpp
    src/parser/parser.hpp
    src/ast/ast.hpp
    src/common.hpp
)

set(ZURA_SOURCE_FILES
    src/main.cpp
    src/ast/ast.cpp
    src/helper/flags.cpp
    src/parser/parser.cpp
    src/lexer/lexer.cpp
)

add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})