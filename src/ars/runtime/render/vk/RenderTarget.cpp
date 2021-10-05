#include "RenderTarget.h"
#include "Context.h"

namespace ars::render::vk {
RenderTargetManager::RenderTargetManager(Context *context)
    : _context(context) {}

Handle<Texture>
RenderTargetManager::get(const RenderTargetManager::Id &id) const {
    auto &rts = _render_targets.get<RTArray>(id);
    auto &state = _render_targets.get<RTState>(id);
    return rts[state.current_index];
}

void RenderTargetManager::update(VkExtent2D reference_size) {
    _reference_size = reference_size;
    _render_targets.for_each_id([&](Id id) { update_rt(id); });
}

void RenderTargetManager::free(const RenderTargetManager::Id &id) {
    _render_targets.free(id);
}

RenderTargetManager::Id RenderTargetManager::alloc(const RenderTargetInfo &info,
                                                   float scale) {
    return alloc(info, [scale](VkExtent2D reference_extent) {
        return VkExtent2D{
            static_cast<uint32_t>(static_cast<float>(reference_extent.width) *
                                  scale),
            static_cast<uint32_t>(static_cast<float>(reference_extent.height) *
                                  scale),
        };
    });
}

RenderTargetManager::Id
RenderTargetManager::alloc(const RenderTargetInfo &info,
                           RenderTargetManager::ScaleFunc func) {
    auto id = _render_targets.alloc();

    auto corrected_info = info;
    corrected_info.multi_buffer_count =
        std::clamp(info.multi_buffer_count, 1u, MAX_RT_MULTI_BUFFER_COUNT);

    _render_targets.get<RenderTargetInfo>(id) = corrected_info;
    _render_targets.get<ScaleFunc>(id) = std::move(func);
    update_rt(id);
    return id;
}

void RenderTargetManager::update_rt(const RenderTargetManager::Id &id) {
    auto &state = _render_targets.get<RTState>(id);

    auto &info = _render_targets.get<RenderTargetInfo>(id);
    state.current_index = (state.current_index + 1) % info.multi_buffer_count;

    auto &rt = _render_targets.get<RTArray>(id)[state.current_index];
    auto &scale_func = _render_targets.get<ScaleFunc>(id);

    auto desired_size = scale_func(_reference_size);
    auto tex_info = info.texture;
    tex_info.extent.width = desired_size.width;
    tex_info.extent.height = desired_size.height;
    tex_info.extent.depth = 1;

    if (rt.get() == nullptr || desired_size.width != rt->info().extent.width ||
        desired_size.height != rt->info().extent.height) {
        rt = _context->create_texture(tex_info);
    }
}
} // namespace ars::render::vk