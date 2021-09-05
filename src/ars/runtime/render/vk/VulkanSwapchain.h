#pragma once

#include "../ISwapchain.h"
#include "../ITexture.h"
#include "Vulkan.h"

namespace ars::render {
class VulkanContext;

class VulkanSwapchain : public ISwapchain {
  public:
    VulkanSwapchain(VulkanContext *context, VkSurfaceKHR surface);
    ~VulkanSwapchain() override;

    void present(ITexture *texture) override;
    void resize(int physical_width, int physical_height) override;

  private:
    VulkanContext *_context = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
};
} // namespace ars::render
