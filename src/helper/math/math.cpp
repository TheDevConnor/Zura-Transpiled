#include "math.hpp"

std::optional<std::string> string_distance(std::vector<std::string> known,
                                           std::string unknown, size_t limit = 3) {
  // Find the smallest levenshtein distance and return
  // the closest match within the limit
  // if there is none within limit, return {}

  size_t min = limit;
  std::string closest = "";
  for (std::string &curr : known) {
    size_t dist = levenshtein_distance(curr, unknown);
    if (dist < min) {
      min = dist;
      closest = curr;
    }
  }

  if (min <= limit && closest != "") {
    return closest;
  }
  // No match found, oh well then
  return {};
}

// Thank you AI, i have no idea what this means!
size_t levenshtein_distance(std::string s1, std::string s2) {
  // Find the levenshtein distance between two strings
  // This is a dynamic programming algorithm
  // that finds the minimum number of operations
  // to transform one string into another

  // Create a 2D array to store the distances
  size_t m = s1.size();
  size_t n = s2.size();
  // vlas are very bad bad
  std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));

  // Initialize the array
  for (size_t i = 0; i <= m; i++) {
    for (size_t j = 0; j <= n; j++) {
      if (i == 0) {
        dp[i][j] = j;
      } else if (j == 0) {
        dp[i][j] = i;
      } else {
        dp[i][j] = 0;
      }
    }
  }

  // Fill in the array
  for (size_t i = 1; i <= m; i++) {
    for (size_t j = 1; j <= n; j++) {
      if (s1[i - 1] == s2[j - 1]) { // only add to the distance if the characters are different
        dp[i][j] = dp[i - 1][j - 1]; // set distance at that location
      } else { // the characters were different
        dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]}); // set distance at that location
      }
    }
  }

  return dp[m][n];
}