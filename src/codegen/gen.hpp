#pragma once

#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"
#include "optimizer/optimize.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <set>

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
    typename std::unordered_map<U, T>::const_iterator iter = map.find(key);
    if (iter == map.end()) {
        return nullptr;
    }
    return iter->second;
}

void initMaps();

// Signed 64-bit integer

// Start at one because retrieving 0(%rbp) results in unusual behavior
inline int64_t variableCount = 8;

// String could be register (%rdi, %rdx, ...) or effective address (-8(%rbp), ...)
inline std::unordered_map<std::string, std::string> variableTable = {};
inline std::vector<size_t> stackSizesForScopes = {}; // wordy term for "when we start a scope, push its stack size"
inline size_t stackSize = 0;
inline std::string insideStructName = "";

void visitStmt(Node::Stmt *stmt);
void visitExpr(Node::Expr *expr);

void symbolType(Node::Type *type);
void arrayType(Node::Type *type);
void pointerType(Node::Type *type);

void importDecl(Node::Stmt *stmt);
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
void linkFile(Node::Stmt *stmt);
void externName(Node::Stmt *stmt);
void matchStmt(Node::Stmt *stmt);

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
void _struct(Node::Expr *expr);
void externalCall(Node::Expr *expr);
void addressExpr(Node::Expr *expr);
void nullExpr(Node::Expr *expr);

void assignStructMember(Node::Expr *expr);
void assignArray(Node::Expr *expr);
void declareStructVariable(Node::Expr *expr, std::string structName, int whereToPut);
void declareArrayVariable(Node::Expr *expr, short int arrayLength, std::string varName);

size_t sizeOfLEB(int64_t value);
int convertFloatToInt(float input); // Float input. Crazy, right?
int round(int num, int multiple); // Round a number up to the nearest multiple

// <name, <type, offset>>
using StructMember = std::pair<std::string, std::pair<Node::Type *, unsigned short int>>;
// <size, <StructMember>>
using Struct = std::pair<size_t, std::vector<StructMember>>;
inline std::unordered_map<std::string, Struct> structByteSizes = {}; // Name of a struct and its size in bytes

signed short int getByteSizeOfType(Node::Type *type); // Return the size of a type in bytes, ie pointers are a size_t (os specific macros baby!)
std::string getUnderlying(Node::Type *type); // Get the underlying type name of a type (ie, int* -> int, []int -> int, int -> int)
std::string type_to_diename(Node::Type *type);

inline std::set<std::string> linkedFiles = {};
inline std::set<std::string> externalNames = {}; // Make sure that when external functions are called, we run "call 'ExternalName'" rather than "call 'usr_FuncName'"/

enum class 
Section {
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
    DIETypes, // this will always be at the end of the regular DIE section
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
inline std::vector<Instr> diet_section = {}; // Acronym for Dwarf Information Entry
inline std::vector<Instr> diea_section = {}; // DIE abbreviation (defines attributes used in DIE's)
inline std::vector<Instr> dies_section = {}; // Dwarf Information Entries strings, referenced from DIE's

inline std::unordered_map<NativeASMFunc, bool> nativeFunctionsUsed = {};
namespace dwarf {
enum class DIEAbbrev {
  Buffer, // 0
  // Compilation unit - always emitted
  // Subprogram (main function should always exist)
  CompileUnit, // 1

  // Function types
  FunctionNoParams,
  FunctionNoParamsVoid,
  FunctionWithParams,
  FunctionWithParamsVoid,

  // Function parameter
  FunctionParam, // aka "Formal Parameter" psssh what garbage bro lmao

  // Variable declaration
  Variable,

  // Block (AKA scope), like while and for, if, etc. Function blocks are different.
  // (... Inline scopes when?)
  LexicalBlock,

  StructType,
  StructMember,

  EnumType, // TAG_enumeration_type (encoding, byte size, type)
  EnumMember, // TAG_enumerator (name, const val)

  // Types
  Type, // EX: char
  PointerType, // EX: char* -> points to a char.
  ArrayType, // EX: [10]char -> array of 10 chars
  ArraySubrange,
  // Void type is not included because it will not be included by function declarations
};
inline std::set<DIEAbbrev> dieAbbrevsUsed = {};
void useAbbrev(DIEAbbrev abbrev);
bool isUsed(DIEAbbrev abbrev);
void useType(Node::Type *type);
void useStringP(std::string what);
inline std::set<std::string> dieNamesUsed = {};
inline std::set<std::string> dieStringsUsed = {};
std::string generateAbbreviations();
inline bool nextBlockDIE = true;

inline static const std::unordered_map<std::string, int> argOP_regs = {
  {"%rax", 0},
  {"%rdx", 1},
  {"%rcx", 2},
  {"%rbx", 3},
  {"%rsi", 4},
  {"%rdi", 5},
  {"%rbp", 6},
  {"%rsp", 7},
  {"%r8", 8},
  {"%r9", 9},
  {"%r10", 10},
  {"%r11", 11},
  {"%r12", 12},
  {"%r13", 13},
  {"%r14", 14},
  {"%r15", 15},
  {"%rip", 16},
  // xmmX registers
  {"%xmm0", 17},
  {"%xmm1", 18},
  {"%xmm2", 19},
  {"%xmm3", 20},
  {"%xmm4", 21},
  {"%xmm5", 22},
  {"%xmm6", 23},
  {"%xmm7", 24},
  {"%xmm8", 25},
  {"%xmm9", 26},
  {"%xmm10", 27},
  {"%xmm11", 28},
  {"%xmm12", 29},
  {"%xmm13", 30},
  {"%xmm14", 31},
  {"%xmm15", 32},
};
}
// Stack count, Variable count
inline std::vector<std::pair<size_t, int64_t>> scopes = {};

inline unsigned char loopDepth = 0;
inline std::vector<std::string> fileIDs = {};
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

int getFileID(const std::string &file);
void pushCompAsExpr(); // assuming compexpr's will already do the "cmp" and "jmp", we will push 0x0 or 0x1 depending on the result

inline const char* file_name;

inline bool debug = false;

// Function argument order
inline static const std::vector<std::string> intArgOrder = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
inline static const std::vector<std::string> floatArgOrder = {"%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5"};

// Helper functions for printing to the console
void prepareSyscallWrite();
void handlePtrType(Node::Expr *arg, PrintStmt *print);
void handleLiteral(Node::Expr *arg);
void handleStrType(Node::Expr *arg);
void handlePrimType(Node::Expr *arg);
void handleFloatType(Node::Expr *arg);

// Helper function to pop the value from the stack to a register
void moveRegister(const std::string &dest, const std::string &src, DataSize dest_size, DataSize src_size);
void popToRegister(const std::string &reg);
void pushRegister(const std::string &reg);
void pushDebug(int line, int file, int column = -1);
void handleExitSyscall();
void handleReturnCleanup();

// Helper for the if statement condition
JumpCondition processComparison(Node::Expr *cond);
int getExpressionDepth(Node::Expr *e);
JumpCondition getOpposite(JumpCondition in);
JumpCondition getJumpCondition(const std::string &op);

bool execute_command(const std::string &command, const std::string &log_file);
void gen(Node::Stmt *stmt, bool isSaved, std::string output, const char* filename, bool isDebug);
void handleError(int line, int pos, std::string msg, std::string typeOfError, bool isFatal = false);
} // namespace codegen