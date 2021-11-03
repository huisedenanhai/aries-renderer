#pragma once

#include <ars/runtime/core/misc/Macro.h>
#include "Vulkan.h"
#include <algorithm>

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

    void set_data_raw(void *value, size_t byte_offset, size_t byte_count);

    template <typename T>
    void set_data(T *value, size_t elem_offset, size_t elem_count) {
        static_assert(std::is_pod_v<T>);
        set_data_raw(reinterpret_cast<void *>(value),
                     elem_offset * sizeof(T),
                     elem_count * sizeof(T));
    }

  private:
    Context *_context = nullptr;
    VkDeviceSize _size = 0;
    VkBuffer _buffer = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;
};
} // namespace ars::render::vk
