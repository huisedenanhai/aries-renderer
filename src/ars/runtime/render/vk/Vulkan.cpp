#include "Vulkan.h"
#include "Context.h"
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

Device::Device(Instance *instance,
               VkDevice device,
               VkPhysicalDevice physical_device)
    : volk::Device(
          device, instance->table().vkGetDeviceProcAddr, instance->allocator()),
      _instance(instance), _physical_device(physical_device) {}

Instance::~Instance() {
    if (_instance != VK_NULL_HANDLE) {
        Destroy();
    }
}

Instance::Instance(VkInstance instance,
                   uint32_t api_version,
                   bool presentation_enabled,
                   const VkAllocationCallbacks *allocator)
    : volk::Instance(instance, allocator),
      _presentation_enabled(presentation_enabled), _api_version(api_version) {}

MemoryView
load_spirv_code(const char *path, const char **flags, uint32_t flag_count) {
    auto code = frill_shaders::load(path, flags, flag_count);
    if (code.code == nullptr) {
        ARS_LOG_CRITICAL("Shader {} not found, have you add it to frill.json?",
                         path);
    }
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
        ARS_LOG_CRITICAL("Failed to alloc command buffer");
    }
}

CommandBuffer::~CommandBuffer() {
    if (_command_buffer != VK_NULL_HANDLE) {
        _device->Free(_pool, 1, &_command_buffer);
    }
}

void CommandBuffer::end() {
    if (_device->EndCommandBuffer(_command_buffer) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to end command buffer");
    }
}

void CommandBuffer::begin(VkCommandBufferUsageFlags usage) {
    VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    info.flags = usage;
    if (_device->BeginCommandBuffer(_command_buffer, &info) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to begin command buffer");
    }
}

} // namespace ars::render::vk
