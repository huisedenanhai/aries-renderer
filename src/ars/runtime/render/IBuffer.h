#pragma once

#include <cstdio>
#include <memory>

namespace ars::render {
class IBuffer {
  public:
    virtual ~IBuffer() = default;
};

struct BufferView {
    IBuffer *buffer;
    // offset and size are specified in bytes
    std::size_t offset;
    std::size_t size;
};
} // namespace ars::render