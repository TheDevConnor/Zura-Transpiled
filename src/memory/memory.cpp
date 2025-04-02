#include "memory.hpp"

void *Allocator::AreanAllocator::alloc(std::size_t size, std::size_t alinment) {
  if ((size > capacity / 4) || (alinment > capacity / 4)) {
    Buffer scratch_buffer = Buffer(size, alinment);

    Buffer old = *buffer;        // store the old ptr
    *buffer = scratch_buffer;    // override the buffer
    buffer->next = old.malloc(); // set the next to the old buffer
    buffer = buffer->next;       // advance the buffer
    return scratch_buffer.ptr;
  }

  while (true) {
    std::size_t aligned_offset = (offset + (alinment - 1)) & ~(alinment - 1);

    if (aligned_offset + size > capacity) {
      // Allocate a new buffer
      if (buffer->next != nullptr) {
        buffer = buffer->next;
        offset = 0;
        continue;
      }

      buffer->next = Buffer(capacity).malloc();
      buffer = buffer->next;
      offset = 0;
      continue;
    }

    void *ptr = buffer->ptr + aligned_offset;
    offset = aligned_offset + size;
    return ptr;
  }
}
