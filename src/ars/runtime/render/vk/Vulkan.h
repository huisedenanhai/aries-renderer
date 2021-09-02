#pragma once

// global configuration for vulkan
// include this header to use vulkan
#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vulkan/volk.hpp>

#include "../../core/misc/NoCopyMove.h"

namespace ars::render {
// By now in this project we only use a single allocator.
//
// These wrappers will take the ownership of vulkan handles and release
// them on destruction.
class VulkanInstance : public volk::Instance {
  public:
    VulkanInstance(VkInstance instance,
                   uint32_t api_version,
                   const VkAllocationCallbacks *allocator = nullptr)
        : volk::Instance(instance, allocator), _api_version(api_version) {}

    ARS_NO_COPY_MOVE(VulkanInstance);

    ~VulkanInstance();

    uint32_t api_version() const {
        return _api_version;
    }

  private:
    uint32_t _api_version = 0;
};

class VulkanDevice : public volk::Device {
  public:
    VulkanDevice(VulkanInstance *instance,
                 VkDevice device,
                 VkPhysicalDevice physical_device)
        : volk::Device(device,
                       instance->table().vkGetDeviceProcAddr,
                       instance->allocator()),
          _instance(instance), _physical_device(physical_device) {}

    ARS_NO_COPY_MOVE(VulkanDevice);

    ~VulkanDevice();

    inline VkPhysicalDevice physical_device() const {
        return _physical_device;
    }

    inline VulkanInstance *instance() const {
        return _instance;
    }

  private:
    VulkanInstance *_instance = nullptr;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
};

struct Queue {
    uint32_t family_index = 0;
    VkQueue queue = VK_NULL_HANDLE;
};

class VulkanMemoryAllocator {
  public:
    explicit VulkanMemoryAllocator(VulkanDevice *device);

    ARS_NO_COPY_MOVE(VulkanMemoryAllocator);

    ~VulkanMemoryAllocator();

    VmaAllocator raw() const;

  private:
    VmaAllocator _allocator;
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

} // namespace ars::render
