#pragma once

#include "../IRenderContext.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Swapchain;

void init_vulkan_backend(const ApplicationInfo &app_info);
void destroy_vulkan_backend();

class Context : public IRenderContext {
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

    [[nodiscard]] Instance *get_instance() const;
    [[nodiscard]] VulkanMemoryAllocator *get_vma() const;

  private:
    void init_device_and_queues(Instance *instance,
                                bool enable_validation,
                                VkSurfaceKHR surface);

    struct Queue {
        uint32_t family_index = 0;
        VkQueue queue = VK_NULL_HANDLE;
    };

    std::unique_ptr<Device> _device{};
    Queue _graphics_queue{};
    Queue _present_queue{};

    std::unique_ptr<VulkanMemoryAllocator> _vma{};

    // a swapchain will be created on context initialization, which should be
    // returned by the next call to create_swapchain with the same window handle
    std::unique_ptr<Swapchain> _cached_swapchain{};
    GLFWwindow *_cached_window{};
};
} // namespace ars::render::vk
