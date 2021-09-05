#pragma once

#include "../IRenderContext.h"
#include "Vulkan.h"

namespace ars::render {
class VulkanSwapchain;

void init_vulkan_backend(const ApplicationInfo &app_info);
void destroy_vulkan_backend();

class VulkanContext : public IRenderContext {
  public:
    // if window is nullptr, presentation will not be supported
    explicit VulkanContext(GLFWwindow *window);

    ARS_NO_COPY_MOVE(VulkanContext);

    ~VulkanContext() override;

    std::unique_ptr<ISwapchain> create_swapchain(GLFWwindow *window) override;
    std::unique_ptr<IBuffer> create_buffer() override;
    std::unique_ptr<ITexture> create_texture() override;
    std::unique_ptr<IScene> create_scene() override;
    std::unique_ptr<IMesh> create_mesh() override;
    std::unique_ptr<IMaterial> create_material() override;

    [[nodiscard]] VulkanInstance *get_instance() const;
    [[nodiscard]] VulkanMemoryAllocator *get_vma() const;

  private:
    void init_device_and_queues(VulkanInstance *instance,
                                bool enable_validation,
                                VkSurfaceKHR surface);

    struct Queue {
        uint32_t family_index = 0;
        VkQueue queue = VK_NULL_HANDLE;
    };

    std::unique_ptr<VulkanDevice> _device{};
    Queue _graphics_queue{};
    Queue _present_queue{};

    std::unique_ptr<VulkanMemoryAllocator> _vma{};

    // a swapchain will be created on context initialization, which should be
    // returned by the next call to create_swapchain with the same window handle
    std::unique_ptr<VulkanSwapchain> _cached_swapchain{};
    GLFWwindow *_cached_window{};
};
} // namespace ars::render
