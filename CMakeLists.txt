cmake_minimum_required(VERSION 3.15)

project(zura LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wpedantic -Wextra -Wundef -Wwrite-strings -Wredundant-decls -Wdisabled-optimization 
                    -Wdouble-promotion -Wmissing-declarations -Wconversion -Wstrict-overflow=2 -fstack-protector-all -Wvla)

# Specify header and source files
set(ZURA_HEADER_FILES
    src/helper/error/error.hpp
    src/helper/flags.hpp
    src/helper/term_color/color.hpp
    src/helper/math/math.hpp

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

    # LSP files
    src/server/lsp.hpp
    src/server/json.hpp

    # Code Gen Files
    src/codegen/optimizer/optimize.hpp
    src/codegen/optimizer/stringify.hpp
    src/codegen/optimizer/compiler.hpp
    src/codegen/optimizer/instr.hpp
    src/codegen/gen.hpp

    src/common.hpp
)

set(ZURA_SOURCE_FILES
    src/helper/error/error.cpp
    src/helper/math/math.cpp
    src/helper/flags.cpp
    src/lexer/lexer.cpp
    src/lexer/maps.cpp

    # Parser Files
    src/parser/helper.cpp
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
    src/typeChecker/helper.cpp

    # LSP files
    src/server/lsp.cpp
    src/server/document.cpp
    src/server/hover.cpp
    src/server/atFunctions.cpp

    # Code Gen Files
    src/codegen/optimizer/optimize.cpp
    src/codegen/optimizer/compiler.cpp
    src/codegen/builtin.cpp
    src/codegen/gen_expr.cpp
    src/codegen/gen_stmt.cpp
    src/codegen/genmap.cpp
    src/codegen/helper.cpp
    src/codegen/gen.cpp
    src/codegen/dwarf.cpp
    
    src/main.cpp
)

# Create executable
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(zura ${ZURA_HEADER_FILES} ${ZURA_SOURCE_FILES})
add_link_options(-lstdc++)
