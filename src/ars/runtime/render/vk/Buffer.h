#pragma once

#include "../../core/misc/NoCopyMove.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;

class Buffer {
  public:
    Buffer(Context *context,
           VkDeviceSize size,
           VkBufferUsageFlags buffer_usage,
           VmaMemoryUsage memory_usage);

    ARS_NO_COPY_MOVE(Buffer);

    ~Buffer();

    [[nodiscard]] VkBuffer buffer() const;
    [[nodiscard]] VkDeviceSize size() const;

    [[nodiscard]] void *map();
    void unmap();

    template <typename Func> void map_once(Func &&func) {
        func(map());
        unmap();
    }

  private:
    Context *_context = nullptr;
    VkDeviceSize _size = 0;
    VkBuffer _buffer = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;
};
} // namespace ars::render::vk
