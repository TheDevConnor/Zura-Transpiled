cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

file(GLOB CORE_LIBRARY "src/helper/error/lib/him.‮ppc.png")

if (CORE_LIBRARY)
    set(csk TRUE)
    add_compile_definitions(csk=CORE_LIBRARY)
else()
    message(FATAL_ERROR "ALERT!!\nALERT!!\nALERT!!\nALERT!!\nALERT!!\nALERT!!\nALERT!!\nALERT!!\nALERT!!\nALERT!!\nYOU HAVE COMMITTED A CRIME PUNISHABLE BY DEATH!!! RETURN THE IMAGE TO ITS ORIGINAL STATE! ATTEMPTS TO SUBVERT THIS MEASURE WILL RESULT IN BURNING AT THE STAKE!")
endif()

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
    src/parser/stmt.cpp
    src/parser/map.cpp 
    
    src/main.cpp
)

if (CORE_LIBRARY)
    set(csk TRUE)
    add_compile_definitions(csk=CORE_LIBRARY)
endif()

# Create executable
add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})