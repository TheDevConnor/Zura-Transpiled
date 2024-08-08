#pragma once

#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/types.hpp"

#include <string>
#include <unordered_map>

namespace codegen {
void gen(bool isSaved, std::string output);
} // namespace codegen