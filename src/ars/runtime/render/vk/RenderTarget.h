#pragma once

#include "Texture.h"
#include "Vulkan.h"
#include <array>
#include <ars/runtime/core/misc/SoA.h>
#include <functional>

namespace ars::render::vk {
class Context;

class RenderTargetManager {
  private:
    using ScaleFunc = std::function<VkExtent2D(VkExtent2D)>;
    struct RTState {
        Handle<Texture> texture;
    };

    using Container = SoA<TextureCreateInfo, ScaleFunc, RTState>;

  public:
    using Id = Container::Id;

    explicit RenderTargetManager(Context *context);

    [[nodiscard]] Handle<Texture> get(const Id &id) const;

    void update(VkExtent2D reference_size);

    // The extent of texture create info is ignored as the actual size is
    // calculated by reference size of RenderTargetManager and scale function of
    // this render target.
    Id alloc(const TextureCreateInfo &info, float scale = 1.0f);
    Id alloc(const TextureCreateInfo &info, ScaleFunc func);

    // All RTs will be automatically freed when RenderTargetManager is destroyed
    void free(const Id &id);

  private:
    void update_rt(const Id &id);

    Context *_context = nullptr;
    Container _render_targets{};
    VkExtent2D _reference_size{};
};

using RenderTargetId = RenderTargetManager::Id;
} // namespace ars::render::vk