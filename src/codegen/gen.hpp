#pragma once

#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"
#include "optimizer/optimize.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace codegen {

using StmtHandler = std::function<void(Node::Stmt *)>;
using ExprHandler = std::function<void(Node::Expr *)>;
using TypeHandler = std::function<void(Node::Type *)>;

inline std::unordered_map<NodeKind, TypeHandler> typeHandlers;
inline std::unordered_map<NodeKind, StmtHandler> stmtHandlers;
inline std::unordered_map<NodeKind, ExprHandler> exprHandlers;
inline std::unordered_map<std::string, std::string> opMap;

template <typename T, typename U>
T lookup(const std::unordered_map<U, T> &map, U key) {
    auto iter = map.find(key);
    if (iter == map.end()) {
        return nullptr;
    }
    return iter->second;
}

void initMaps();

// Signed 64-bit integer

// Start at one because retrieving 0(%rbp) results in unusual behavior
inline int64_t variableCount = 1;

// String could be register (%rdi, %rdx, ...) or effective address (-8(%rbp), ...)
inline std::unordered_map<std::string, std::string> variableTable = {};
inline std::unordered_map<std::string, std::unordered_map<std::string, std::string>> structTable = {};
inline std::vector<size_t> stackSizesForScopes = {}; // wordy term for "when we start a scope, push its stack size"
inline size_t stackSize;

void visitStmt(Node::Stmt *stmt);
void visitExpr(Node::Expr *expr);

void symbolType(Node::Type *type);
void arrayType(Node::Type *type);
void pointerType(Node::Type *type);

void structDecl(Node::Stmt *stmt);
void enumDecl(Node::Stmt *stmt);
void program(Node::Stmt *stmt);
void constDecl(Node::Stmt *stmt);
void funcDecl(Node::Stmt *stmt);
void varDecl(Node::Stmt *stmt);
void block(Node::Stmt *stmt);
void whileLoop(Node::Stmt *stmt);
void forLoop(Node::Stmt *stmt);
void ifStmt(Node::Stmt *stmt);
void print(Node::Stmt *stmt);
void expr(Node::Stmt *stmt);
void _break(Node::Stmt *stmt);
void _continue(Node::Stmt *stmt);
void _return(Node::Stmt *stmt);

void _arrayExpr(Node::Expr *expr);
void arrayElem(Node::Expr *expr);
void binary(Node::Expr *expr);
void grouping(Node::Expr *expr);
void unary(Node::Expr *expr);
void call(Node::Expr *expr);
void ternary(Node::Expr *expr);
void assign(Node::Expr *expr);
void primary(Node::Expr *expr);
void cast(Node::Expr *expr);
void memberExpr(Node::Expr *expr);

int convertFloatToInt(float input); // Float input. Crazy, right?

// assembly
enum class 
Section { // BRUH its 10:13 ok ibrb // Connor brb grabing some ice cream
    // section .text
    Main, // main function
    Head, // user functions
    // section .data
    Data,
    // section .rodata
    ReadonlyData,

    DIE, // Dwarf Information Entries - basically, they're little stinkbombs that tell the debugger what variable its looking at
    DIEString, // String literals referenced by DIE's
    DIEAbbrev, // Abbreviations for DIE's
};

enum class NativeASMFunc {
    strlen,
    itoa,
};

inline std::vector<Instr> text_section = {};
inline std::vector<Instr> head_section = {};
inline std::vector<Instr> data_section = {}; // This is only really one of three instructions
inline std::vector<Instr> rodt_section = {}; // Same as data section
inline std::vector<Instr> die_section = {}; // Acronym for Dwarf Information Entry
inline std::vector<Instr> diea_section = {}; // DIE abbreviation (defines attributes used in DIE's)
inline std::vector<Instr> dies_section = {}; // Dwarf Information Entries strings, referenced from DIE's

inline std::unordered_map<NativeASMFunc, bool> nativeFunctionsUsed = {};

// Stack count, Variable count
inline std::vector<std::pair<size_t, int64_t>> scopes = {};

inline bool isEntryPoint = false;
inline size_t dieCount = 1; // Labels! Labels galore! Im not counting bytes, man! Let LD do it !!!
inline size_t howBadIsRbp = 0;
inline size_t conditionalCount = 0;
inline size_t stringCount = 0;
inline size_t floatCount = 0;
inline size_t loopCount = 0;
inline size_t arrayCount = 0;
//                            idx    # ELEM
inline std::vector<std::pair<size_t, size_t>> arrayCounts = {};

inline size_t funcBlockStart = -1; // Set to "stackSize" on FuncDecl blocks, will be set to crazy value when not used
void push(Instr instr, Section section = Section::Main);
void pushLinker(std::string val, Section section);

void pushCompAsExpr(); // assuming compexpr's will already do the "cmp" and "jmp", we will push 0x0 or 0x1 depending on the result

inline const char* file_name;

inline bool debug = false;

// Function argument order
inline static const std::vector<std::string> argOrder = {"%rsi","%rdx","%rcx","%r8","%r9"};

// Helper function to pop the value from the stack to a register
void moveRegister(const std::string &dest, const std::string &src, DataSize dest_size, DataSize src_size);
void popToRegister(const std::string &reg);
void pushRegister(const std::string &reg);
void pushDebug(int line);
void handleExitSyscall();
void handleReturnCleanup();

// Helper for the if statement condition
void processBinaryExpression(BinaryExpr *cond, const std::string &preconCount, const std::string &name, bool isLoop = false);
int getExpressionDepth(BinaryExpr *e);
JumpCondition getJumpCondition(const std::string &op);

bool execute_command(const std::string &command, const std::string &log_file);
void gen(Node::Stmt *stmt, bool isSaved, std::string output, const char* filename, bool isDebug);
void handlerError(int line, int pos, std::string msg, std::string typeOfError);
} // namespace codegen