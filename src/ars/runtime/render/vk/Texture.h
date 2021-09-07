#pragma once

#include "../ITexture.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;

struct TextureCreateInfo {
    VkImageType image_type;
    VkImageViewType view_type;
    VkFormat format;
    VkExtent3D extent;
    uint32_t mip_levels;
    uint32_t array_layers;
    VkSampleCountFlagBits samples;
    VkImageUsageFlags usage;
    VkImageAspectFlags aspect_mask;

    static TextureCreateInfo sampled_2d(VkFormat format,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t mip_levels);
};

class Texture : public ITexture {
  public:
    Texture(Context *context, const TextureCreateInfo &info);

    ~Texture() override;

  private:
    [[nodiscard]] VkImageSubresourceRange get_subresource_range() const;

    Context *_context = nullptr;
    VmaAllocation _allocation = VK_NULL_HANDLE;
    VkImage _image = VK_NULL_HANDLE;
    VkImageView _image_view = VK_NULL_HANDLE;

    TextureCreateInfo _info{};
};
} // namespace ars::render::vk
