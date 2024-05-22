cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

# Specify header and source files
set(ZURA_HEADER_FILES
    src/helper/error/error.hpp
    src/helper/flags.hpp 
    src/lexer/lexer.hpp
    src/lexer/maps.hpp
    inc/colorize.hpp
    inc/update.hpp
    src/common.hpp
)

set(ZURA_SOURCE_FILES
    src/helper/flags.cpp
    src/lexer/lexer.cpp
    src/main.cpp
)

# Create executable
add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})



