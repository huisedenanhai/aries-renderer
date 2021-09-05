#pragma once

#include <memory>

namespace ars::render {
class ITexture;

class ISwapchain {
  public:
    // blit a 2d texture to screen and swap buffer
    virtual void present(ITexture *texture) = 0;
    virtual void resize(int physical_width, int physical_height) = 0;

    virtual ~ISwapchain() = default;
};
} // namespace ars::render