#include "Vulkan.h"

#include <stdexcept>

namespace ars::render {
VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanDevice *device) {
    auto instance = device->instance();
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.vulkanApiVersion = instance->api_version();
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

VulkanDevice::~VulkanDevice() {
    if (_device != VK_NULL_HANDLE) {
        DeviceWaitIdle();
        Destroy();
    }
}

VulkanInstance::~VulkanInstance() {
    if (_instance != VK_NULL_HANDLE) {
        Destroy();
    }
}
} // namespace ars::render
