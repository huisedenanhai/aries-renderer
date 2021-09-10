#pragma once

#include "../IContext.h"
#include "Texture.h"
#include "Vulkan.h"

#include <vector>

namespace ars::render::vk {
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
    [[nodiscard]] VkQueue raw() const;

    void submit(CommandBuffer *command_buffer);
    // these command buffers will be freed when the next frame begins
    CommandBuffer *alloc_tmp_command_buffer(VkCommandBufferLevel level);

    // record a command buffer and submit it immediately
    template <typename Func> void submit_immediate(Func &&func);

    // wait the queue idle
    void flush();
    // tmp buffers will be freed by this method
    void reset_command_pool();

  private:
    void init_command_pool();

    // A queue for synchronizing each command buffer submissions
    std::vector<VkSemaphore> _semaphores;
    Context *_context = nullptr;
    uint32_t _family_index = 0;
    VkQueue _queue = VK_NULL_HANDLE;

    VkCommandPool _command_pool = VK_NULL_HANDLE;
    std::vector<std::unique_ptr<CommandBuffer>> _tmp_command_buffers{};
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

    bool begin_frame() override;
    void end_frame() override;

  private:
    // this method init device if not
    std::unique_ptr<Swapchain> create_swapchain_impl(GLFWwindow *window);

    void init_device_and_queues(Instance *instance,
                                bool enable_validation,
                                VkSurfaceKHR surface);

    std::unique_ptr<Device> _device{};
    std::unique_ptr<Queue> _queue{};

    std::unique_ptr<VulkanMemoryAllocator> _vma{};

    // a swapchain will be created on context initialization, which should
    // be returned by the next call to create_swapchain with the same window
    // handle
    std::unique_ptr<Swapchain> _cached_swapchain{};
    GLFWwindow *_cached_window{};
};

template <typename Func> void Queue::submit_immediate(Func &&func) {
    auto cmd = alloc_tmp_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    func(cmd);
    cmd->end();
    submit(cmd);
}

} // namespace ars::render::vk
