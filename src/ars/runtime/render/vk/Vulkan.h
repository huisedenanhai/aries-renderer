#pragma once

// global configuration for vulkan
// include this header to use vulkan
#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vulkan/volk.hpp>
#include <vector>
#include <ars/runtime/core/misc/Macro.h>
#include <memory>
#include <string>

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

    void begin_sample(const std::string &name,
                      uint32_t color,
                      const char *file_name,
                      uint32_t line,
                      const char *function_name);
    void end_sample();

    void begin_debug_label(const std::string &name, uint32_t color);
    void end_debug_label();

    Context *context() const;

  private:
    Context *_context = nullptr;
    VkCommandPool _pool = VK_NULL_HANDLE;
};

#define ARS_DEBUG_LABEL_VK(cmd, name, color)                                   \
    (cmd)->begin_debug_label(name, color);                                     \
    ARS_DEFER_TAGGED(vk_debug_label, [&]() { (cmd)->end_debug_label(); })

// A reference counted handle for resources.
// The context uses the handle for resource usage tracking and delayed resource
// destruction.
template <typename T> struct Handle {
  public:
    Handle() = default;
    Handle(std::nullptr_t) : _ptr(nullptr) {}

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

    bool operator==(const Handle &rhs) const {
        return _ptr == rhs._ptr;
    }

    bool operator!=(const Handle &rhs) const {
        return _ptr != rhs._ptr;
    }

  private:
    friend Context;

    explicit Handle(std::shared_ptr<T> ptr) : _ptr(std::move(ptr)) {}

    std::shared_ptr<T> _ptr = nullptr;
};

constexpr VkAccessFlags VULKAN_ACCESS_WRITE_MASK =
    VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT |
    VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT |
    VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT |
    VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
    VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV |
    VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;

constexpr VkAccessFlags VULKAN_ACCESS_READ_MASK =
    VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
    VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
    VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
    VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT |
    VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
    VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
    VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
    VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
    VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV |
    VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT |
    VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV |
    VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV |
    VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;

template <typename T> struct VulkanObjectType;

template <>
struct VulkanObjectType<VkPipeline>
    : std::integral_constant<VkObjectType, VK_OBJECT_TYPE_PIPELINE> {};

template <>
struct VulkanObjectType<VkPipelineLayout>
    : std::integral_constant<VkObjectType, VK_OBJECT_TYPE_PIPELINE_LAYOUT> {};

template <>
struct VulkanObjectType<VkRenderPass>
    : std::integral_constant<VkObjectType, VK_OBJECT_TYPE_RENDER_PASS> {};

template <>
struct VulkanObjectType<VkImage>
    : std::integral_constant<VkObjectType, VK_OBJECT_TYPE_IMAGE> {};

template <>
struct VulkanObjectType<VkImageView>
    : std::integral_constant<VkObjectType, VK_OBJECT_TYPE_IMAGE_VIEW> {};

template <>
struct VulkanObjectType<VkSampler>
    : std::integral_constant<VkObjectType, VK_OBJECT_TYPE_SAMPLER> {};

// Connect a sequence of vulkan struct with pNext.
// The input struct must start with field sType and pNext
void set_up_vk_struct_chain(const std::vector<void *> &chain);
} // namespace ars::render::vk
