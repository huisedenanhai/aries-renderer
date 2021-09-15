#include "Buffer.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {

Buffer::Buffer(Context *context,
               VkDeviceSize size,
               VkBufferUsageFlags buffer_usage,
               VmaMemoryUsage memory_usage)
    : _context(context), _size(size) {
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = buffer_usage;

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage = memory_usage;

    if (vmaCreateBuffer(_context->vma()->raw(),
                        &buffer_info,
                        &alloc_info,
                        &_buffer,
                        &_allocation,
                        nullptr) != VK_SUCCESS) {
        panic("Failed to create buffer");
    }
}

Buffer::~Buffer() {
    if (_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(_context->vma()->raw(), _buffer, _allocation);
    }
}

void *Buffer::map() {
    void *mapped_data;
    if (vmaMapMemory(_context->vma()->raw(), _allocation, &mapped_data) !=
        VK_SUCCESS) {
        panic("Failed to map buffer memory");
    }
    return mapped_data;
}

void Buffer::unmap() {
    vmaUnmapMemory(_context->vma()->raw(), _allocation);
}

VkBuffer Buffer::buffer() const {
    return _buffer;
}

VkDeviceSize Buffer::size() const {
    return _size;
}

void Buffer::set_data_raw(void *value, size_t byte_offset, size_t byte_count) {
    if (byte_offset >= _size) {
        return;
    }

    map_once([&](void *ptr) {
        auto t_ptr = reinterpret_cast<uint8_t *>(ptr);
        auto count =
            std::min(byte_count, static_cast<size_t>(_size) - byte_offset);
        std::memcpy(t_ptr + byte_offset, value, count);
    });
}
} // namespace ars::render::vk
