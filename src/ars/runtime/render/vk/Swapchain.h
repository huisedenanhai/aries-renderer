#pragma once

#include "../ISwapchain.h"
#include "../ITexture.h"
#include "Vulkan.h"
#include <vector>

namespace ars::render::vk {
class Context;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
};

SwapChainSupportDetails query_swapchain_support(
    Instance *instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface);

class Swapchain : public ISwapchain {
  public:
    Swapchain(Context *context,
              VkSurfaceKHR surface,
              int physical_width,
              int physical_height);
    ~Swapchain() override;

    ARS_NO_COPY_MOVE(Swapchain);

    void present(ITexture *texture) override;
    void resize(int physical_width, int physical_height) override;

  private:
    void cleanup_swapchain();
    void init_swapchain(int physical_width, int physical_height);
    void init_image_views();

    Context *_context = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

    std::vector<VkImage> _swapchain_images{};
    std::vector<VkImageView> _swapchain_image_views{};
    VkFormat _swapchain_image_format{};
    VkExtent2D _swapchain_extent{};
    VkImageSubresourceRange _swapchain_subresource_range{};
};
} // namespace ars::render::vk
