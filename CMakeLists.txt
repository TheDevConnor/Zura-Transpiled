cmake_minimum_required(VERSION 3.15)

project(zura)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(ZURA_HEADER_FILES
    src/helper/error/error.hpp
    src/parser/parser.hpp
    src/helper/flags.hpp
    src/lexer/lexer.hpp
    # src/type/type.hpp
    inc/colorize.hpp
    src/ast/ast.hpp
    inc/update.hpp
    src/common.hpp
    )
    
    set(ZURA_SOURCE_FILES
    src/parser/parser.cpp
    src/helper/flags.cpp
    # src/type/detType.cpp
    src/parser/expr.cpp
    src/parser/stmt.cpp
    src/lexer/lexer.cpp
    src/parser/def.cpp
    # src/type/type.cpp
    src/ast/ast.cpp
    src/main.cpp
)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(zura ${llvm_libs})
