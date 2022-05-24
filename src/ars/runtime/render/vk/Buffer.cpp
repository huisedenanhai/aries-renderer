#include "Buffer.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {

Buffer::Buffer(Context *context,
               VkDeviceSize size,
               VkBufferUsageFlags buffer_usage,
               VmaMemoryUsage memory_usage)
    : _context(context), _size(size), _buffer_usage(buffer_usage),
      _memory_usage(memory_usage) {
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

    if (_memory_usage == VMA_MEMORY_USAGE_GPU_ONLY) {
        auto staging_buffer =
            _context->create_buffer(byte_count,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
        staging_buffer->set_data_raw(value, 0, byte_count);
        _context->queue()->submit_once([&](CommandBuffer *cmd) {
            VkBufferCopy copy{};
            copy.srcOffset = 0;
            copy.dstOffset = byte_offset;
            copy.size = byte_count;
            cmd->CopyBuffer(staging_buffer->buffer(), _buffer, 1, &copy);
        });
        return;
    }

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

Handle<Buffer> Heap::buffer() const {
    return _buffer;
}

uint64_t Heap::alloc(VkDeviceSize size) {
    auto required_capacity = _size + size;
    if (required_capacity > _buffer->size()) {
        extend(static_cast<VkDeviceSize>(required_capacity +
                                         (required_capacity >> 2)));
    }
    auto ptr = _size;
    _size += size;
    return ptr;
}

void Heap::free(uint64_t ptr) {
    // TODO correct alloc/free
}

Heap::Heap(Context *context, const HeapInfo &info)
    : _context(context), _info(info) {
    // Just waste 128 bits, so we can always return none zero offset
    _size = 128;
    extend(1024);
}

void Heap::extend(VkDeviceSize capacity) {
    if (_buffer != nullptr && _buffer->size() >= capacity) {
        return;
    }

    auto new_buffer = _context->create_buffer(
        capacity, _info.buffer_usage, _info.memory_usage);
    if (_buffer != nullptr) {
        assert(_size <= capacity);
        _context->queue()->submit_once([&](CommandBuffer *cmd) {
            VkBufferCopy copy{};
            copy.size = _size;
            cmd->CopyBuffer(_buffer->buffer(), new_buffer->buffer(), 1, &copy);
        });
    }
    _buffer = new_buffer;
}

Handle<HeapRangeOwned> Heap::alloc_owned(VkDeviceSize size) {
    auto offset = alloc(size);
    return _context->create_heap_range_owned(this, offset, size);
}

HeapInfo HeapInfo::named(NamedHeap heap) {
    assert(heap < NamedHeap_Count);
    HeapInfo info{};
    info.buffer_usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (heap) {
    case NamedHeap_Vertices:
        info.buffer_usage |=
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
        info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case NamedHeap_Indices:
        info.buffer_usage |=
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
        info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
        break;
    case NamedHeap_Count:
        break;
    }
    return info;
}

HeapRangeOwned::~HeapRangeOwned() {
    _heap->free(_offset);
}

HeapRangeOwned::HeapRangeOwned(Heap *heap, uint64_t offset, VkDeviceSize size)
    : _heap(heap), _offset(offset), _size(size) {}

Heap *HeapRangeOwned::heap() const {
    return _heap;
}

uint64_t HeapRangeOwned::offset() const {
    return _offset;
}

VkDeviceSize HeapRangeOwned::size() const {
    return _size;
}

HeapRange HeapRangeOwned::slice() const {
    return slice(0, size());
}

HeapRange HeapRangeOwned::slice(uint64_t offset_in_range,
                                VkDeviceSize size) const {
    HeapRange range{};
    range.heap = heap();
    range.offset = _offset + offset_in_range;
    range.size = size;
    return range;
}

HeapRange HeapRange::slice() const {
    return slice(0, size);
}

HeapRange HeapRange::slice(uint64_t offset_in_range,
                           VkDeviceSize new_size) const {
    HeapRange range{};
    range.heap = heap;
    range.offset = offset + offset_in_range;
    range.size = new_size;
    return range;
}

void HeapRange::set_data_raw(void *value,
                             size_t byte_offset,
                             size_t byte_count) {
    if (heap == nullptr) {
        return;
    }
    heap->buffer()->set_data_raw(value, offset + byte_offset, byte_count);
}
} // namespace ars::render::vk
