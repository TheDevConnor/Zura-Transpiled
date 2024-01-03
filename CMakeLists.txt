cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

set(ZURA_HEADER_FILES
    src/helper/error/lexerError.hpp
    src/ast/generation/codeGen.hpp
    src/parser/parser.hpp
    src/helper/flags.hpp
    src/lexer/lexer.hpp
    src/type/type.hpp
    inc/colorize.hpp
    src/ast/ast.hpp
    inc/update.hpp
    src/common.hpp
    )
    
    set(ZURA_SOURCE_FILES
    src/ast/generation/codeGen.cpp
    src/parser/parser.cpp
    src/helper/flags.cpp
    src/type/detType.cpp
    src/parser/expr.cpp
    src/parser/stmt.cpp
    src/lexer/lexer.cpp
    src/type/error.cpp
    src/parser/def.cpp
    src/type/type.cpp
    src/ast/ast.cpp
    src/main.cpp
)

add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})
