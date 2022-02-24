#pragma once

#include "Vulkan.h"
#include <algorithm>
#include <ars/runtime/core/misc/Macro.h>

namespace ars::render::vk {
class Context;

namespace details {
template <typename T> struct BufferDataViewTrait {
  public:
    // If one really what to pass in something like device pointer, he must cast
    // it to integer or wrap it in some struct
    static_assert(std::is_pod_v<T> && !std::is_pointer_v<T>);
    static void *ptr(const T &v) {
        return (void *)&v;
    }

    static size_t size(const T &v) {
        return sizeof(T);
    }
};

template <typename T> struct BufferDataViewTrait<std::vector<T>> {
  public:
    static_assert(std::is_pod_v<T> && !std::is_pointer_v<T>);
    static void *ptr(const std::vector<T> &v) {
        return (void *)(v.data());
    }

    static size_t size(const std::vector<T> &v) {
        return sizeof(T) * v.size();
    }
};
} // namespace details

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
    void set_data_array(const T *value, size_t elem_offset, size_t elem_count) {
        static_assert(std::is_pod_v<T> && !std::is_pointer_v<T>);
        set_data_raw(
            (void *)(value), elem_offset * sizeof(T), elem_count * sizeof(T));
    }

    template <typename T> void set_data(const T &value) {
        using DataView = details::BufferDataViewTrait<T>;
        set_data_raw(DataView::ptr(value), 0, DataView::size(value));
    }

  private:
    Context *_context = nullptr;
    VkDeviceSize _size = 0;
    VkBuffer _buffer = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;
};
} // namespace ars::render::vk
