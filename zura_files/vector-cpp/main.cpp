#include "vector.hpp" // adjust the path if needed
#include <iostream>

int main() {
  Vector<int> vec;

  // Initialize vector
  vec.init();

  // Test push_back
  std::cout << "Pushing values: ";
  for (int i = 1; i <= 4; ++i) {
    vec.push_back(i);
    std::cout << i << " ";
  }
  std::cout << "\n";

  // Print current size and capacity
  std::cout << "Size after push: " << vec.get_size() << "\n";
  std::cout << "Capacity after push: " << vec.get_capacity() << "\n";

  // Test get_index
  std::cout << "Values in vector: ";
  for (std::size_t i = 0; i < vec.get_size(); ++i) {
    std::cout << *vec.get_index(i) << " ";
  }
  std::cout << "\n";

  // Test pop_back
  vec.pop_back();
  std::cout << "Size after pop: " << vec.get_size() << "\n";

  // Test out-of-bounds access
  try {
    std::cout << "Accessing out-of-bounds index...\n";
    vec.get_index(100);
  } catch (const std::out_of_range &e) {
    std::cout << "Caught exception: " << e.what() << "\n";
  }

  // Test popping until empty
  std::cout << "Popping all elements...\n";
  while (vec.get_size() > 0) {
    vec.pop_back();
    std::cout << "Size now: " << vec.get_size() << "\n";
  }

  // Test popping from empty vector
  try {
    std::cout << "Attempting pop on empty vector...\n";
    vec.pop_back();
  } catch (const std::out_of_range &e) {
    std::cout << "Caught exception: " << e.what() << "\n";
  }

  // Free resources
  vec.free();

  return 0;
}
