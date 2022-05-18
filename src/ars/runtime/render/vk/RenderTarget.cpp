#include "RenderTarget.h"
#include "Context.h"

namespace ars::render::vk {
RenderTargetManager::RenderTargetManager(Context *context)
    : _context(context) {}

Handle<Texture>
RenderTargetManager::get(const RenderTargetManager::Id &id) const {
    auto &state = _render_targets.get<RTState>(id);
    return state.texture;
}

void RenderTargetManager::update(VkExtent2D reference_size) {
    _reference_size = reference_size;
    _render_targets.for_each_id([&](Id id) { update_rt(id); });

    bool any_rt_resized = false;
    _render_targets.for_each_id([&](Id id) {
        if (resized(id)) {
            any_rt_resized = true;
        }
    });

    if (any_rt_resized) {
        _context->queue()->submit_once([&](CommandBuffer *cmd) {
            _render_targets.for_each_id([&](Id id) {
                if (resized(id)) {
                    auto tex = _render_targets.get<RTState>(id).texture;
                    tex->transfer_layout(cmd,
                                         VK_IMAGE_LAYOUT_GENERAL,
                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                         0,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         VK_ACCESS_TRANSFER_WRITE_BIT);
                    auto range = tex->subresource_range();
                    if (range.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) {
                        VkClearColorValue value{};
                        cmd->ClearColorImage(
                            tex->image(), tex->layout(), &value, 1, &range);
                    } else if ((range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
                               (range.aspectMask &
                                VK_IMAGE_ASPECT_STENCIL_BIT)) {
                        VkClearDepthStencilValue value{};
                        cmd->ClearDepthStencilImage(
                            tex->image(), tex->layout(), &value, 1, &range);
                    }
                }
            });
        });
    }
}

void RenderTargetManager::free(const RenderTargetManager::Id &id) {
    _render_targets.free(id);
}

RenderTargetManager::Id
RenderTargetManager::alloc(const TextureCreateInfo &info, float scale) {
    return alloc(info, [scale](VkExtent2D reference_extent) {
        auto scale_value = [&](uint32_t v) {
            return std::max(
                static_cast<uint32_t>(static_cast<float>(v) * scale), 0u);
        };
        return VkExtent2D{scale_value(reference_extent.width),
                          scale_value(reference_extent.height)};
    });
}

RenderTargetManager::Id
RenderTargetManager::alloc(const TextureCreateInfo &info,
                           RenderTargetManager::ScaleFunc func) {
    auto id = _render_targets.alloc();

    _render_targets.get<TextureCreateInfo>(id) = info;
    _render_targets.get<ScaleFunc>(id) = std::move(func);
    update_rt(id);
    return id;
}

void RenderTargetManager::update_rt(const RenderTargetManager::Id &id) {
    auto &state = _render_targets.get<RTState>(id);
    state.resized = false;

    auto &rt = state.texture;
    auto &scale_func = _render_targets.get<ScaleFunc>(id);

    auto desired_size = scale_func(_reference_size);
    desired_size.width = std::max(desired_size.width, 1u);
    desired_size.height = std::max(desired_size.height, 1u);

    auto tex_info = _render_targets.get<TextureCreateInfo>(id);
    tex_info.extent.width = desired_size.width;
    tex_info.extent.height = desired_size.height;
    tex_info.extent.depth = 1;
    // All rt can be transfer dst. This is needed as we will clear it on
    // creating.
    tex_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (rt.get() == nullptr || desired_size.width != rt->info().extent.width ||
        desired_size.height != rt->info().extent.height) {
        rt = _context->create_texture(tex_info);
        state.resized = true;
    }
}

bool RenderTargetManager::resized(const RenderTargetManager::Id &id) {
    return _render_targets.get<RTState>(id).resized;
}
} // namespace ars::render::vk