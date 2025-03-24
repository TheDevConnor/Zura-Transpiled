#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <optional>
#include <limits>

// distance algorithm, used for finding the smallest distance of unknown ident and known ident
size_t levenshtein_distance(std::string s1, std::string s2);
std::optional<std::string> string_distance(std::vector<std::string> known, std::string unknown, size_t limit);