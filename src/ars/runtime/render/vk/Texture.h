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

    VkFilter min_filter = VK_FILTER_LINEAR;
    VkFilter mag_filter = VK_FILTER_LINEAR;
    VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    static TextureCreateInfo sampled_2d(VkFormat format,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t mip_levels);
};

VkFormat translate(render::Format format);

TextureCreateInfo translate(const render::TextureInfo &info);

class Texture {
  public:
    Texture(Context *context, const TextureCreateInfo &info);

    ARS_NO_COPY_MOVE(Texture);

    ~Texture();

    void set_data(void *data,
                  size_t size,
                  uint32_t mip_level,
                  uint32_t layer,
                  int32_t x_offset,
                  int32_t y_offset,
                  int32_t z_offset,
                  uint32_t x_size,
                  uint32_t y_size,
                  uint32_t z_size);

    void generate_mipmap();

    [[nodiscard]] VkSampler sampler() const;
    [[nodiscard]] VkImageLayout layout() const;
    [[nodiscard]] VkImageView image_view() const;

    [[nodiscard]] const TextureCreateInfo &info() const;

  private:
    // Transfer all subresources to the target layout and set the _layout field.
    // This method assumes all layers and levels of the image are in the same
    // layout, and public methods should maintain this invariance.
    void transfer_layout(CommandBuffer *cmd,
                         VkImageLayout dst_layout,
                         VkShaderStageFlags src_stage_mask,
                         VkAccessFlags src_access_mask,
                         VkShaderStageFlags dst_stage_mask,
                         VkAccessFlags dst_access_mask);

    [[nodiscard]] VkImageSubresourceRange get_subresource_range() const;

    Context *_context = nullptr;
    VmaAllocation _allocation = VK_NULL_HANDLE;
    VkImage _image = VK_NULL_HANDLE;
    VkImageView _image_view = VK_NULL_HANDLE;
    VkSampler _sampler = VK_NULL_HANDLE;
    // Public methods should ensure all layers and levels of the image are in
    // the same layout
    VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;

    TextureCreateInfo _info{};
};

class TextureAdapter : public ITexture {
  public:
    TextureAdapter(const TextureInfo &info, Handle<Texture> texture);

    ARS_NO_COPY_MOVE(TextureAdapter);

    void set_data(void *data,
                  size_t size,
                  uint32_t mip_level,
                  uint32_t layer,
                  int32_t x_offset,
                  int32_t y_offset,
                  int32_t z_offset,
                  uint32_t x_size,
                  uint32_t y_size,
                  uint32_t z_size) override;

    void generate_mipmap() override;

    [[nodiscard]] Handle<Texture> texture() const;

  private:
    Handle<Texture> _texture{};
};

Handle<Texture> upcast(ITexture *texture);
} // namespace ars::render::vk