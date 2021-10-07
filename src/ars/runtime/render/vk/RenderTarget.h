#pragma once

#include "Texture.h"
#include "Vulkan.h"
#include <array>
#include <ars/runtime/core/misc/SoA.h>
#include <functional>

namespace ars::render::vk {
class Context;

constexpr uint32_t MAX_RT_MULTI_BUFFER_COUNT = 3;

struct RenderTargetInfo {
    // The extent of texture create info is ignored as the actual size is
    // calculated by reference size of RenderTargetManager and scale function of
    // this render target.
    TextureCreateInfo texture{};
    uint32_t multi_buffer_count = 1;
};

class RenderTargetManager {
  private:
    using ScaleFunc = std::function<VkExtent2D(VkExtent2D)>;
    using RTArray = std::array<Handle<Texture>, MAX_RT_MULTI_BUFFER_COUNT>;

    struct RTState {
        uint32_t current_index = 0;
    };

    using Container = SoA<RenderTargetInfo, ScaleFunc, RTArray, RTState>;

  public:
    using Id = Container::Id;

    explicit RenderTargetManager(Context *context);

    [[nodiscard]] Handle<Texture> get(const Id &id) const;

    void update(VkExtent2D reference_size);

    Id alloc(const RenderTargetInfo &info, float scale = 1.0f);

    // All RTs will be automatically freed when RenderTargetManager is destroyed
    void free(const Id &id);

  private:
    Id alloc(const RenderTargetInfo &info, ScaleFunc func);
    void update_rt(const Id &id);

    Context *_context = nullptr;
    Container _render_targets{};
    VkExtent2D _reference_size{};
};

using RenderTargetId = RenderTargetManager::Id;
} // namespace ars::render::vk