cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Display LLVM information
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include directories and definitions
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Specify LLVM libraries
set(llvm_libs LLVM)

# Specify header and source files
set(ZURA_HEADER_FILES
    src/helper/error/error.hpp
    src/parser/parser.hpp
    src/helper/flags.hpp
    src/lexer/lexer.hpp
    inc/colorize.hpp
    src/ast/ast.hpp
    inc/update.hpp
    src/common.hpp
)

set(ZURA_SOURCE_FILES
    src/parser/parser.cpp
    src/helper/flags.cpp
    src/parser/expr.cpp
    src/parser/stmt.cpp
    src/lexer/lexer.cpp
    src/parser/def.cpp
    src/ast/ast.cpp
    src/main.cpp
)

# Create executable
add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})

# Link LLVM libraries
target_link_libraries(zura ${llvm_libs})


