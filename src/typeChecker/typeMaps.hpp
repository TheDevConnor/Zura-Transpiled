#pragma once

#include <string>
#include <unordered_map>

#include "../ast/ast.hpp"
#include "type.hpp"

struct NameAndType {
  std::string name;
  Node::Type* type;

  bool operator==(const NameAndType& other) const {
      return name == other.name && type == other.type;
  }
};

// Specialization of std::hash for NameAndType
namespace std {
  template <>
  struct hash<NameAndType> {
      size_t operator()(const NameAndType& nt) const noexcept {
          size_t h1 = hash<string>{}(nt.name);
          size_t h2 = hash<Node::Type*>()(nt.type);
          return h1 ^ (h2 << 1); // Bitwise combination
      }
  };
}

struct ParamAndType {
  std::unordered_map<std::string, Node::Type *> params;
};

struct SymbolTable : std::unordered_map<std::string, Node::Type *> {
    int line, pos;
    bool contains(const std::string &name) const {
        return find(name) != end();
    }

    void declare(const std::string &name, Node::Type *type) {
        if (!contains(name)) {
            insert({name, type});
            return;
        }
        std::string msg = "Variable already declared: " + name;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
    }

    Node::Type *lookup(const std::string &name) const {
        return at(name);
    }
};


struct FunctionTable : std::unordered_map<NameAndType, ParamAndType> {
  int line, pos;
  bool contains(const NameAndType &nameAndType) {
    return find(nameAndType) != end();
  }

  void declare(const std::string &name, std::unordered_map<std::string, Node::Type *> params, Node::Type *returnType) {
    std::cout << name << std::endl;
    if (name == "main" && params.size() == 0) TypeChecker::foundMain = true; 
    if (!contains({name, returnType})) {
        insert({{name, returnType}, {params}});
        return;
    }
    std::string msg = "Function already declared: " + name;
    TypeChecker::handleError(line, pos, msg, "", "Type Error");
  }

  ParamAndType getParams(const std::string &name, Node::Type *type) {
    return at({name, type});
  }

  ParamAndType lookup(const std::string &name, Node::Type *type) {
    return at({name, type});
  }
};

struct StructTable : std::unordered_map<std::string, std::unordered_map<std::string, std::pair<Node::Type *, std::unordered_map<std::string, Node::Type *>>>> {
    int line, pos;
    bool contains(const std::string &name) {
        return find(name) != end();
    }

    void declare(const std::string &name) {
        if (!contains(name)) {
            insert({name, {}});
            return;
        }
        std::string msg = "Struct already declared: " + name;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
    }

    void addMember(const std::string &structName, const std::string &memberName, Node::Type *type) {
        if (contains(structName)) {
            at(structName).insert({memberName, {type, {}}});
            return;
        }
        std::string msg = "Struct not declared: " + structName;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
    }

    void addFunction(const std::string &structName, const std::string &memberName, Node::Type *type, std::unordered_map<std::string, Node::Type *> params) {
        // structs[structName].methods.declare(methodName, returnType, params);
        if (contains(structName)) {
            // check if the member is already declared
            if (at(structName).find(memberName) != at(structName).end()) {
                std::string msg = "Method already declared: " + memberName;
                TypeChecker::handleError(line, pos, msg, "", "Type Error");
                return;
            }
            at(structName).insert({memberName, {type, params}});
            return;
        }
        std::string msg = "Struct not declared: " + structName;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
    }

    Node::Type *lookup(const std::string &structName, const std::string &memberName) {
        if (contains(structName)) {
            auto it = at(structName).find(memberName);
            if (it != at(structName).end()) {
                return it->second.first;
            }
            std::string msg = "Member not found: " + memberName;
            TypeChecker::handleError(line, pos, msg, "", "Type Error");
            return new SymbolType("unknown");
        }
        std::string msg = "Struct not declared: " + structName;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
        return new SymbolType("unknown");
    }
};

struct EnumTable : std::unordered_map<std::string, std::unordered_map<std::string, int>> {
    int line, pos;
    bool contains(const std::string &name) {
        return find(name) != end();
    }

    void declare(const std::string &name) {
        if (!contains(name)) {
            insert({name, {}});
            return;
        }
        std::string msg = "Enum already declared: " + name;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
    }

    void addMember(const std::string &enumName, const std::string &memberName, int position) {
        if (contains(enumName)) {
            at(enumName)[memberName] = position;
            return;
        }
        std::string msg = "Enum not declared: " + enumName;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
    }

    int lookup(const std::string &enumName, const std::string &memberName) {
        if (contains(enumName)) {
            auto it = at(enumName).find(memberName);
            if (it != at(enumName).end()) {
                return it->second;
            }
            std::string msg = "Member not found: " + memberName;
            TypeChecker::handleError(line, pos, msg, "", "Type Error");
            return -1;
        }
        std::string msg = "Enum not declared: " + enumName;
        TypeChecker::handleError(line, pos, msg, "", "Type Error");
        return -1;
    }
};

class TypeCheckerContext {
public:
  SymbolTable globalSymbols;
  std::vector<SymbolTable> localScopes;
  FunctionTable functionTable;
  StructTable structTable;
  EnumTable enumTable;
  std::vector<std::string> stackKeys;

  void enterScope() { localScopes.emplace_back(); }

  void exitScope() {
    if (!localScopes.empty()) {
      localScopes.pop_back();
    }
  }

  Node::Type *lookup(const std::string &name) {
    for (auto it = localScopes.rbegin(); it != localScopes.rend(); ++it) {
      if (it->contains(name))
        return it->lookup(name);
    }
    return globalSymbols.lookup(name);
  }

  void declareGlobal(const std::string &name, Node::Type *type) {
    globalSymbols.declare(name, type);
  }

  void declareLocal(const std::string &name, Node::Type *type) {
    if (!localScopes.empty()) {
      localScopes.back().declare(name, type);
    }
  }
};

inline std::unique_ptr<TypeCheckerContext> context = nullptr;
