#pragma once

#include "Common.h"
#include <memory>

namespace ars::render {
class ITexture;

struct WindowInfo {
    const char *title = nullptr;
    Extent2D logical_size{800, 600};
};

class IWindow {
  public:
    // Blit a 2d texture to screen and swap buffer.
    virtual void present(ITexture *texture) = 0;

    virtual Extent2D physical_size() = 0;

    virtual bool should_close() = 0;

    virtual ~IWindow() = default;
};
} // namespace ars::render