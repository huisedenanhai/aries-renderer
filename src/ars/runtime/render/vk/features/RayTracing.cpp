#include "RayTracing.h"
#include "../Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
AccelerationStructure::AccelerationStructure(
    Context *context,
    VkAccelerationStructureTypeKHR type,
    VkDeviceSize buffer_size)
    : _context(context) {
    _buffer = _context->create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkAccelerationStructureCreateInfoKHR info{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    info.type = type;
    info.buffer = _buffer->buffer();
    info.offset = 0;
    info.size = buffer_size;

    auto device = _context->device();
    if (device->CreateAccelerationStructureKHR(
            &info, &_acceleration_structure) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create BLAS");
    }
}

AccelerationStructure::~AccelerationStructure() {
    if (_acceleration_structure != VK_NULL_HANDLE) {
        _context->device()->Destroy(_acceleration_structure);
    }
}

VkAccelerationStructureKHR
AccelerationStructure::acceleration_structure() const {
    return _acceleration_structure;
}
} // namespace ars::render::vk