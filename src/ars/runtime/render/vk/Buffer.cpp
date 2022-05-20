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
        ARS_LOG_CRITICAL("Failed to create buffer");
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
        ARS_LOG_CRITICAL("Failed to map buffer memory");
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
    if (value == nullptr) {
        return;
    }

    assert(byte_offset + byte_count <= _size);

    map_once([&](void *ptr) {
        auto t_ptr = reinterpret_cast<uint8_t *>(ptr);
        std::memcpy(t_ptr + byte_offset, value, byte_count);
    });
}

VkDeviceAddress Buffer::device_address() const {
    if (_context->info().device_address_features.bufferDeviceAddress ==
        VK_FALSE) {
        return 0;
    }
    VkBufferDeviceAddressInfo info{
        VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
    info.buffer = _buffer;
    return _context->device()->GetBufferDeviceAddressKHR(&info);
}
} // namespace ars::render::vk
