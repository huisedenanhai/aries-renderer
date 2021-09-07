#pragma once

#include "../IContext.h"
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

    [[nodiscard]] uint32_t family_index() const;
    [[nodiscard]] VkQueue raw() const;

  private:
    Context *_context = nullptr;
    uint32_t _family_index = 0;
    VkQueue _queue = VK_NULL_HANDLE;
};

class Context : public IContext {
  public:
    // if window is nullptr, presentation will not be supported
    explicit Context(GLFWwindow *window);

    ARS_NO_COPY_MOVE(Context);

    ~Context() override;

    std::unique_ptr<ISwapchain> create_swapchain(GLFWwindow *window) override;
    std::unique_ptr<IBuffer> create_buffer() override;
    std::unique_ptr<ITexture> create_texture() override;
    std::unique_ptr<IScene> create_scene() override;
    std::unique_ptr<IMesh> create_mesh() override;
    std::unique_ptr<IMaterial> create_material() override;

    [[nodiscard]] Instance *instance() const;
    [[nodiscard]] Device *device() const;
    [[nodiscard]] VulkanMemoryAllocator *vma() const;

    [[nodiscard]] Queue *graphics_queue() const;
    [[nodiscard]] Queue *present_queue() const;
    [[nodiscard]] std::vector<uint32_t> get_unique_queue_family_indices() const;

  private:
    // this method init device if not
    std::unique_ptr<Swapchain> create_swapchain_impl(GLFWwindow *window);

    void init_device_and_queues(Instance *instance,
                                bool enable_validation,
                                VkSurfaceKHR surface);

    std::unique_ptr<Device> _device{};
    std::unique_ptr<Queue> _graphics_queue{};
    std::unique_ptr<Queue> _present_queue{};

    std::unique_ptr<VulkanMemoryAllocator> _vma{};

    // a swapchain will be created on context initialization, which should be
    // returned by the next call to create_swapchain with the same window handle
    std::unique_ptr<Swapchain> _cached_swapchain{};
    GLFWwindow *_cached_window{};
};
} // namespace ars::render::vk
