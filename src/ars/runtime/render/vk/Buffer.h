#pragma once

#include "Vulkan.h"
#include <algorithm>
#include <ars/runtime/core/misc/Macro.h>
#include <vector>

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

// Implement utility methods for setting data as long as set_data_raw method is
// defined in the derived type B.
template <typename B> struct BufferSetDataMethods {
    template <typename T>
    void set_data_array(const T *value, size_t elem_offset, size_t elem_count) {
        static_assert(std::is_pod_v<T> && !std::is_pointer_v<T>);
        static_cast<B *>(this)->set_data_raw(
            (void *)(value), elem_offset * sizeof(T), elem_count * sizeof(T));
    }

    template <typename T> void set_data(const T &value) {
        using DataView = details::BufferDataViewTrait<T>;
        static_cast<B *>(this)->set_data_raw(
            DataView::ptr(value), 0, DataView::size(value));
    }
};
} // namespace details

class Buffer : public details::BufferSetDataMethods<Buffer> {
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

    // Only valid when VK_KHR_buffer_device_address is available
    [[nodiscard]] VkDeviceAddress device_address() const;

  private:
    Context *_context = nullptr;
    VkDeviceSize _size = 0;
    VkBuffer _buffer = VK_NULL_HANDLE;
    VmaAllocation _allocation = VK_NULL_HANDLE;
    VkBufferUsageFlags _buffer_usage{};
    VmaMemoryUsage _memory_usage{};
};

enum NamedHeap {
    NamedHeap_Vertices,
    NamedHeap_Indices,
    NamedHeap_Count,
};

struct HeapInfo {
    VkBufferUsageFlags buffer_usage;
    VmaMemoryUsage memory_usage;

    static HeapInfo named(NamedHeap heap);
};

class Heap;

struct HeapRange : details::BufferSetDataMethods<HeapRange> {
    Heap *heap = nullptr;
    uint64_t offset = 0;
    VkDeviceSize size = 0;

    HeapRange slice() const;
    HeapRange slice(uint64_t offset_in_range, VkDeviceSize size) const;
    void set_data_raw(void *value, size_t byte_offset, size_t byte_count);
    VkDeviceAddress device_address() const;
    VkBuffer buffer() const;
};

class HeapRangeOwned {
  public:
    HeapRangeOwned(Heap *heap, uint64_t offset, VkDeviceSize size);
    ~HeapRangeOwned();

    ARS_NO_COPY_MOVE(HeapRangeOwned);

    Heap *heap() const;
    uint64_t offset() const;
    VkDeviceSize size() const;
    HeapRange slice() const;
    HeapRange slice(uint64_t offset_in_range, VkDeviceSize size) const;

  private:
    Heap *_heap = nullptr;
    uint64_t _offset = 0;
    VkDeviceSize _size = 0;
};

class Heap {
  public:
    Heap(Context *context, const HeapInfo &info);

    Handle<Buffer> buffer() const;
    uint64_t alloc(VkDeviceSize size);
    Handle<HeapRangeOwned> alloc_owned(VkDeviceSize size);
    void free(uint64_t ptr);

  private:
    void extend(VkDeviceSize capacity);

    Context *_context = nullptr;
    HeapInfo _info{};
    Handle<Buffer> _buffer{};
    uint64_t _size = 0;
};

} // namespace ars::render::vk
