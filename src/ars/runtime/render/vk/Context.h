#pragma once

#include "../IContext.h"
#include "Buffer.h"
#include "Descriptor.h"
#include "Texture.h"
#include "Vulkan.h"

#include <vector>

namespace ars::render::vk {
class DescriptorArena;
class Swapchain;
class Context;

void init_vulkan_backend(const ApplicationInfo &app_info);
void destroy_vulkan_backend();

// Owned by the context
class Queue {
  public:
    Queue(Context *context, uint32_t family_index);

    ARS_NO_COPY_MOVE(Queue);

    ~Queue();

    [[nodiscard]] uint32_t family_index() const;
    [[nodiscard]] VkQueue queue() const;

    void submit(CommandBuffer *command_buffer,
                uint32_t wait_semaphore_count = 0,
                VkSemaphore *wait_semaphores = nullptr);

    // record a command buffer and submit it immediately
    template <typename Func>
    void submit_once(Func &&func,
                     uint32_t wait_semaphore_count = 0,
                     VkSemaphore *wait_semaphores = nullptr);

    // The signaled semaphore for the last submitted command.
    // If the queue is flushed, this method returns VK_NULL_HANDLE
    [[nodiscard]] VkSemaphore get_semaphore() const;

    // wait the queue idle
    void flush();

  private:
    // A queue for synchronizing each command buffer submissions
    std::vector<VkSemaphore> _semaphores;
    Context *_context = nullptr;
    uint32_t _family_index = 0;
    VkQueue _queue = VK_NULL_HANDLE;
};

struct ContextProperties {
    bool anisotropic_sampler_enabled = false;
    float max_sampler_anisotropy = 1.0f;
};

class Context : public IContext {
  public:
    // if window is nullptr, presentation will not be supported
    explicit Context(GLFWwindow *window);

    ARS_NO_COPY_MOVE(Context);

    ~Context() override;

    std::unique_ptr<ISwapchain> create_swapchain(GLFWwindow *window) override;
    std::unique_ptr<ITexture>
    create_texture_impl(const TextureInfo &info) override;
    std::unique_ptr<IScene> create_scene() override;
    std::unique_ptr<IMesh> create_mesh() override;
    std::unique_ptr<IMaterial> create_material() override;

    [[nodiscard]] Instance *instance() const;
    [[nodiscard]] Device *device() const;
    [[nodiscard]] VulkanMemoryAllocator *vma() const;
    [[nodiscard]] VkPipelineCache pipeline_cache() const;

    // The primary queue.
    //
    // This queue supports all operations: graphics, compute, transfer,
    // and presentation (if presentation is required).
    //
    // The spec says: if there are any queue supports graphics operations, there
    // must be a queue family supports both graphics and compute operations, and
    // transfer operations are implicitly supported if either graphics or
    // compute operations are supported.
    // (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkQueueFlagBits.html)
    //
    // It will be rare for us to find no such queue.
    [[nodiscard]] Queue *queue() const;

    [[nodiscard]] DescriptorArena *descriptor_arena() const;

    [[nodiscard]] const ContextProperties &properties() const;

    bool begin_frame() override;
    void end_frame() override;

    Handle<Texture> create_texture(const TextureCreateInfo &info);
    Handle<CommandBuffer> create_command_buffer(VkCommandBufferLevel level);
    Handle<Buffer> create_buffer(VkDeviceSize size,
                                 VkBufferUsageFlags buffer_usage,
                                 VmaMemoryUsage memory_usage);

  private:
    // This method init device if not
    std::unique_ptr<Swapchain> create_swapchain_impl(GLFWwindow *window);

    void init_device_and_queues(Instance *instance,
                                bool enable_validation,
                                VkSurfaceKHR surface);
    void init_command_pool();
    void init_pipeline_cache();
    void init_descriptor_arena();

    // Clear unused resources
    void gc();

    template <typename T, typename... Args>
    static Handle<T> create_handle(std::vector<std::shared_ptr<T>> &pool,
                                   Args &&...args);

    std::unique_ptr<Device> _device{};
    std::unique_ptr<Queue> _queue{};

    std::unique_ptr<VulkanMemoryAllocator> _vma{};

    std::unique_ptr<DescriptorArena> _descriptor_arena{};

    // a swapchain will be created on context initialization, which should
    // be returned by the next call to create_swapchain with the same window
    // handle
    std::unique_ptr<Swapchain> _cached_swapchain{};
    GLFWwindow *_cached_window{};

    VkCommandPool _command_pool = VK_NULL_HANDLE;

    VkPipelineCache _pipeline_cache = VK_NULL_HANDLE;

    // cached resources
    std::vector<std::shared_ptr<Texture>> _textures{};
    std::vector<std::shared_ptr<CommandBuffer>> _command_buffers{};
    std::vector<std::shared_ptr<Buffer>> _buffers{};

    ContextProperties _properties{};
};

template <typename Func>
void Queue::submit_once(Func &&func,
                        uint32_t wait_semaphore_count,
                        VkSemaphore *wait_semaphores) {
    auto cmd = _context->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    func(cmd.get());
    cmd->end();
    submit(cmd.get(), wait_semaphore_count, wait_semaphores);
}

} // namespace ars::render::vk
