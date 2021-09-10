#include "Vulkan.h"
#include <ars/runtime/core/Log.h>
#include <frill_shaders.hpp>
#include <stdexcept>

namespace ars::render::vk {
VulkanMemoryAllocator::VulkanMemoryAllocator(Device *device) : _device(device) {
    auto instance = device->instance();
    VmaAllocatorCreateInfo allocator_info{};

    // just use the default api version, which is Vulkan 1.0
    allocator_info.vulkanApiVersion = 0;
    allocator_info.physicalDevice = device->physical_device();
    allocator_info.device = device->device();
    allocator_info.instance = instance->instance();

    const auto &table = device->table();

    VmaVulkanFunctions funcs{
        instance->table().vkGetPhysicalDeviceProperties,
        instance->table().vkGetPhysicalDeviceMemoryProperties,
        table.vkAllocateMemory,
        table.vkFreeMemory,
        table.vkMapMemory,
        table.vkUnmapMemory,
        table.vkFlushMappedMemoryRanges,
        table.vkInvalidateMappedMemoryRanges,
        table.vkBindBufferMemory,
        table.vkBindImageMemory,
        table.vkGetBufferMemoryRequirements,
        table.vkGetImageMemoryRequirements,
        table.vkCreateBuffer,
        table.vkDestroyBuffer,
        table.vkCreateImage,
        table.vkDestroyImage,
        table.vkCmdCopyBuffer,
    };

    allocator_info.pVulkanFunctions = &funcs;

    if (vmaCreateAllocator(&allocator_info, &_allocator) != VK_SUCCESS) {
        throw std::runtime_error("failed to create allocator");
    }
}

VulkanMemoryAllocator::~VulkanMemoryAllocator() {
    vmaDestroyAllocator(_allocator);
}

VmaAllocator VulkanMemoryAllocator::raw() const {
    return _allocator;
}

Device::~Device() {
    if (_device != VK_NULL_HANDLE) {
        DeviceWaitIdle();
        Destroy();
    }
}

Instance::~Instance() {
    if (_instance != VK_NULL_HANDLE) {
        Destroy();
    }
}

MemoryView
load_spirv_code(const char *path, const char **flags, uint32_t flag_count) {
    auto code = frill_shaders::load(path, flags, flag_count);
    return MemoryView{code.code, code.size};
}

CommandBuffer::CommandBuffer(Device *device,
                             VkCommandPool pool,
                             VkCommandBufferLevel level)
    : volk::CommandBuffer(device, VK_NULL_HANDLE), _pool(pool) {
    VkCommandBufferAllocateInfo info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandBufferCount = 1;
    info.commandPool = pool;
    info.level = level;
    if (device->Allocate(&info, &_command_buffer) != VK_SUCCESS) {
        panic("Failed to alloc command buffer");
    }
}

CommandBuffer::~CommandBuffer() {
    if (_command_buffer != VK_NULL_HANDLE) {
        _device->Free(_pool, 1, &_command_buffer);
    }
}

void CommandBuffer::reset() {
    _device->ResetCommandBuffer(_command_buffer, 0);
}
} // namespace ars::render::vk
