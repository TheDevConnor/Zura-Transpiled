#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>

#include <new>

// NOTE: remove any small buffers after resize.

namespace Allocator {
struct Buffer {
  std::size_t size = 0;
  Buffer *next = 0;
  std::byte *ptr = 0;

  Buffer(std::size_t s, std::size_t alignment = 1024) {
    alignment = std::min(s, alignment);
    ptr = static_cast<std::byte *>(std::aligned_alloc(alignment, s));
    size = s;
  }

  Buffer *malloc() {
    Buffer *new_buffer = static_cast<Buffer *>(std::malloc(sizeof(Buffer)));
    *new_buffer = *this;
    return new_buffer;
  }
};

class AreanAllocator {
public:
  explicit AreanAllocator(std::size_t size) : capacity(size) {
    buffer = Buffer(size).malloc();
    head = buffer;
    if (!buffer)
      throw std::bad_alloc();
  }

  void *alloc(std::size_t size,
              std::size_t alinment = alignof(std::max_align_t));

  template <typename T, typename... Args> T *emplace(Args &&...args) {
    auto p = static_cast<T *>(alloc(sizeof(T), alignof(T)));
    new (p) T(std::forward<Args>(args)...);
    return p;
  }

  void reset() {
    offset = 0;
    buffer = head;
  }

  ~AreanAllocator() {
    reset();

    while (buffer != nullptr) {
      auto next = buffer->next;
      free(buffer->ptr);
      free(buffer);
      buffer = next;
    }
  }

private:
  std::size_t capacity = 0;
  std::size_t offset = 0;
  Buffer *buffer = 0;
  Buffer *head = 0;
};
}; // namespace Allocator
