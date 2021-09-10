#pragma once

// global configuration for vulkan
// include this header to use vulkan
#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vulkan/volk.hpp>

#include "../../core/misc/NoCopyMove.h"

namespace ars::render::vk {
// By now in this project we only use a single allocator.
//
// These wrappers will take the ownership of vulkan handles and release
// them on destruction.
class Instance : public volk::Instance {
  public:
    Instance(VkInstance instance,
             uint32_t api_version,
             bool presentation_enabled,
             const VkAllocationCallbacks *allocator = nullptr)
        : volk::Instance(instance, allocator),
          _presentation_enabled(presentation_enabled),
          _api_version(api_version) {}

    ARS_NO_COPY_MOVE(Instance);

    ~Instance();

    [[nodiscard]] uint32_t api_version() const {
        return _api_version;
    }

    [[nodiscard]] bool presentation_enabled() const {
        return _presentation_enabled;
    };

  private:
    uint32_t _api_version = 0;
    bool _presentation_enabled = false;
};

class Device : public volk::Device {
  public:
    Device(Instance *instance,
           VkDevice device,
           VkPhysicalDevice physical_device)
        : volk::Device(device,
                       instance->table().vkGetDeviceProcAddr,
                       instance->allocator()),
          _instance(instance), _physical_device(physical_device) {}

    ARS_NO_COPY_MOVE(Device);

    ~Device();

    [[nodiscard]] inline VkPhysicalDevice physical_device() const {
        return _physical_device;
    }

    [[nodiscard]] inline Instance *instance() const {
        return _instance;
    }

  private:
    Instance *_instance = nullptr;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
};

class VulkanMemoryAllocator {
  public:
    explicit VulkanMemoryAllocator(Device *device);

    ARS_NO_COPY_MOVE(VulkanMemoryAllocator);

    ~VulkanMemoryAllocator();

    [[nodiscard]] VmaAllocator raw() const;

  private:
    Device *_device = nullptr;
    VmaAllocator _allocator = VK_NULL_HANDLE;
};

class CommandBuffer : public volk::CommandBuffer {
  public:
    CommandBuffer(Device *device,
                  VkCommandPool pool,
                  VkCommandBufferLevel level);

    void reset();

    ~CommandBuffer();

  private:
    VkCommandPool _pool;
};

struct MemoryView {
    const uint8_t *data;
    size_t size;
};

// Return nullptr if shader is not found.
// When the shader exists, this method return the pointer to the SPIRV code
// and the size will be set correctly
MemoryView
load_spirv_code(const char *path, const char **flags, uint32_t flag_count);

} // namespace ars::render::vk
