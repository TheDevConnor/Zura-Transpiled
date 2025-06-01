#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename T> class Vector {
public:
  void init() {
    data = nullptr;
    size = 0;
    capacity = 0;
  }

  void free() {
    delete[] data;
    data = nullptr;
    size = 0;
    capacity = 0;
  }

  void push_back(T &value) {
    if (size == capacity) {
      std::size_t new_capacity = (capacity == 0) ? 1 : capacity * 2;
      realloc(new_capacity);
    }
    data[size++] = value;
  }

  void pop_back() {
    if (size == 0)
      throw std::out_of_range("Pop from empty vector!");
    size--;
  }

  T *get_index(std::size_t index) {
    if (index >= size)
      throw std::out_of_range("Index out of range!");
    return &data[index];
  }

  std::size_t get_size() { return size; }
  std::size_t get_capacity() { return capacity; }

private:
  T *data;
  std::size_t size;
  std::size_t capacity;

  void realloc(std::size_t new_capacity) {
    T *new_data = new T[new_capacity];
    for (std::size_t i = 0; i < size; i++)
      new_data[i] = std::move(data[i]);
    delete[] data;
    data = new_data;
    capacity = new_capacity;
  }
};
