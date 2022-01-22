#pragma once

// global configuration for vulkan
// include this header to use vulkan
#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vulkan/volk.hpp>

#include <ars/runtime/core/misc/Macro.h>
#include <memory>

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
             const VkAllocationCallbacks *allocator = nullptr);

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
           VkPhysicalDevice physical_device);

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

struct MemoryView {
    const uint8_t *data;
    size_t size;
};

// Return nullptr if shader is not found.
// When the shader exists, this method return the pointer to the SPIRV code
// and the size will be set correctly
MemoryView
load_spirv_code(const char *path, const char **flags, uint32_t flag_count);

class Context;

class CommandBuffer : public volk::CommandBuffer {
  public:
    CommandBuffer(Context *context,
                  VkCommandPool pool,
                  VkCommandBufferLevel level);

    ARS_NO_COPY_MOVE(CommandBuffer);

    ~CommandBuffer();

    void begin(VkCommandBufferUsageFlags usage);

    void end();

    void begin_sample(const std::string &name, uint32_t color);
    void end_sample();

    Context *context() const;

  private:
    Context *_context = nullptr;
    VkCommandPool _pool = VK_NULL_HANDLE;
};

// A reference counted handle for resources.
// The context uses the handle for resource usage tracking and delayed resource
// destruction.
template <typename T> struct Handle {
  public:
    Handle() = default;

    [[nodiscard]] T *get() const {
        return _ptr.get();
    }

    T *operator->() const {
        return _ptr.get();
    }

    bool operator==(std::nullptr_t) const {
        return _ptr == nullptr;
    }

    bool operator!=(std::nullptr_t) const {
        return _ptr != nullptr;
    }

  private:
    friend Context;

    explicit Handle(std::shared_ptr<T> ptr) : _ptr(std::move(ptr)) {}

    std::shared_ptr<T> _ptr = nullptr;
};

} // namespace ars::render::vk
