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
              uint32_t physical_width,
              uint32_t physical_height);
    ~Swapchain() override;

    ARS_NO_COPY_MOVE(Swapchain);

    void present(ITexture *texture) override;
    void resize(uint32_t physical_width, uint32_t physical_height) override;
    Extent2D get_size() override;

  private:
    void set_target_size(uint32_t physical_width, uint32_t physical_height);
    bool need_recreate() const;
    // Recreation of swapchain does not change surface format after first call
    // for initialization.
    void recreate_swapchain();
    void cleanup_swapchain();
    void init_swapchain();
    void init_image_views();
    void init_render_pass();
    void init_framebuffers();

    Context *_context = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

    std::vector<VkImage> _images{};
    std::vector<VkImageView> _image_views{};
    VkSurfaceFormatKHR _format{};
    VkExtent2D _extent{};
    VkImageSubresourceRange _subresource_range{};

    // Blit the image using shader
    VkRenderPass _render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers{};

    // lazy swapchain recreation
    VkExtent2D _target_extent{};
};
} // namespace ars::render::vk
