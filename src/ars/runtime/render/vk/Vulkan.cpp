#include "Vulkan.h"
#include "Context.h"
#include "Profiler.h"
#include <ars/runtime/core/Log.h>
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

CommandBuffer::CommandBuffer(Context *context,
                             VkCommandPool pool,
                             VkCommandBufferLevel level)
    : volk::CommandBuffer(context->device(), VK_NULL_HANDLE), _context(context),
      _pool(pool) {
    VkCommandBufferAllocateInfo info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandBufferCount = 1;
    info.commandPool = pool;
    info.level = level;
    if (_device->Allocate(&info, &_command_buffer) != VK_SUCCESS) {
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

Context *CommandBuffer::context() const {
    return _context;
}

void CommandBuffer::begin_sample(const std::string &name,
                                 uint32_t color,
                                 const char *file_name,
                                 uint32_t line,
                                 const char *function_name) {
    auto profiler = _context->profiler();
    if (profiler != nullptr) {
        profiler->begin_sample(
            this, name, color, file_name, line, function_name);
    }
}

void CommandBuffer::end_sample() {
    auto profiler = _context->profiler();
    if (profiler != nullptr) {
        profiler->end_sample(this);
    }
}

void CommandBuffer::begin_debug_label(const std::string &name, uint32_t color) {
    _context->begin_debug_label(this, name, color);
}

void CommandBuffer::end_debug_label() {
    _context->end_debug_label(this);
}

namespace {
struct VulkanStructHeader {
    VkStructureType sType;
    const void *pNext;
};
} // namespace

void set_up_vk_struct_chain(const std::vector<void *> &chain) {
    VulkanStructHeader *cur = nullptr;
    for (auto e : chain) {
        if (e == nullptr) {
            continue;
        }
        if (cur != nullptr) {
            cur->pNext = e;
        }
        cur = reinterpret_cast<VulkanStructHeader *>(e);
    }
    if (cur != nullptr) {
        cur->pNext = nullptr;
    }
}
} // namespace ars::render::vk
