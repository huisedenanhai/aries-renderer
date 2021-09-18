#pragma once

#include "Common.h"
#include <functional>
#include <memory>
#include <optional>

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

    virtual void
    set_imgui_callback(std::optional<std::function<void()>> callback) = 0;

    virtual ~IWindow() = default;
};
} // namespace ars::render