#pragma once

#include "../IRenderContext.h"
#include "Vulkan.h"

namespace ars::render {
class VulkanSwapchain;

void init_vulkan_backend(const ApplicationInfo &app_info);

class VulkanContext : public IRenderContext {
  public:
    VulkanContext();

    ARS_NO_COPY_MOVE(VulkanContext);

    std::unique_ptr<ISwapchain> create_swapchain(GLFWwindow *window) override;
    std::unique_ptr<IBuffer> create_buffer() override;
    std::unique_ptr<ITexture> create_texture() override;
    std::unique_ptr<IScene> create_scene() override;
    std::unique_ptr<IMesh> create_mesh() override;
    std::unique_ptr<IMaterial> create_material() override;

  private:
    std::unique_ptr<VulkanDevice> _device{};
    // a swapchain will be created on context initialization, which should be
    // returned by the next call to create_swapchain with the same window handle
    std::unique_ptr<VulkanSwapchain> _cached_swapchain{};
    uint64_t _cached_window_handle{};
};
} // namespace ars::render
