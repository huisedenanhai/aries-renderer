#include "Texture.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
Texture::Texture(Context *context, const TextureCreateInfo &info)
    : _context(context), _info(info) {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.arrayLayers = info.array_layers;
    image_info.imageType = info.image_type;
    image_info.mipLevels = info.mip_levels;
    image_info.samples = info.samples;
    image_info.extent = info.extent;
    image_info.format = info.format;
    image_info.usage = info.usage;

    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    if (info.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
        info.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
        image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    auto queues = context->get_unique_queue_family_indices();
    if (queues.size() <= 1) {
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        image_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        image_info.queueFamilyIndexCount = static_cast<uint32_t>(queues.size());
        image_info.pQueueFamilyIndices = queues.data();
    }

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto vma = context->vma();
    if (vmaCreateImage(vma->raw(),
                       &image_info,
                       &alloc_info,
                       &_image,
                       &_allocation,
                       nullptr) != VK_SUCCESS) {
        panic("Failed to create image");
    }

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = _image;
    view_info.format = info.format;
    view_info.viewType = info.view_type;
    view_info.subresourceRange = get_subresource_range();
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (context->device()->Create(&view_info, &_image_view) != VK_SUCCESS) {
        panic("Failed to create image view");
    }
}

Texture::~Texture() {
    if (_image != VK_NULL_HANDLE) {
        vmaDestroyImage(_context->vma()->raw(), _image, _allocation);
    }
    if (_image_view != VK_NULL_HANDLE) {
        _context->device()->Destroy(_image_view);
    }
}

VkImageSubresourceRange Texture::get_subresource_range() const {
    VkImageSubresourceRange range{};
    range.aspectMask = _info.aspect_mask;
    range.baseArrayLayer = 0;
    range.layerCount = _info.array_layers;
    range.baseMipLevel = 0;
    range.levelCount = _info.mip_levels;

    return range;
}

TextureCreateInfo TextureCreateInfo::sampled_2d(VkFormat format,
                                                uint32_t width,
                                                uint32_t height,
                                                uint32_t mip_levels) {
    TextureCreateInfo info{};
    info.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.view_type = VK_IMAGE_VIEW_TYPE_2D;
    info.image_type = VK_IMAGE_TYPE_2D;
    info.format = format;
    info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.mip_levels = mip_levels;
    info.array_layers = 1;
    info.extent = VkExtent3D{width, height, 1};

    return info;
}

TextureAdapter::TextureAdapter(const TextureInfo &info,
                               std::shared_ptr<Texture> texture)
    : ITexture(info), _texture(std::move(texture)) {}

Texture *TextureAdapter::texture() const {
    return _texture.get();
}

VkFormat translate(render::Format format) {
    switch (format) {
    case Format::R8G8B8A8Srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    }
}

TextureCreateInfo translate(const TextureInfo &info) {
    TextureCreateInfo up{};
    up.format = translate(info.format);
    switch (info.type) {
    case TextureType::Texture2D:
        up.image_type = VK_IMAGE_TYPE_2D;
        up.view_type = VK_IMAGE_VIEW_TYPE_2D;
        break;
    }
    up.array_layers = info.array_layers;
    up.mip_levels = info.mip_levels;
    up.extent.width = info.width;
    up.extent.height = info.height;
    up.extent.depth = info.depth;
    up.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    up.samples = VK_SAMPLE_COUNT_1_BIT;
    up.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    return up;
}
} // namespace ars::render::vk
