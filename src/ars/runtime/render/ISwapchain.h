#pragma once

#include "Common.h"
#include <memory>

namespace ars::render {
class ITexture;

class ISwapchain {
  public:
    // Blit a 2d texture to screen and swap buffer
    virtual void present(ITexture *texture) = 0;
    virtual void resize(uint32_t physical_width, uint32_t physical_height) = 0;
    // The target sizeof swapchain may differ from the one set by resize(), for
    // platform specific reason.
    virtual Extent2D get_size() = 0;

    virtual ~ISwapchain() = default;
};
} // namespace ars::render