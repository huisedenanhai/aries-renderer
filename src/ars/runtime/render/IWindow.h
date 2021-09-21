#pragma once

#include "Common.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace ars::render {
class ITexture;

struct WindowInfo {
    std::string title{};
    Extent2D logical_size{800, 600};
};

class IWindow {
  public:
    // Blit a 2d texture to screen and swap buffer.
    virtual void present(ITexture *texture) = 0;

    virtual Extent2D physical_size() = 0;

    virtual bool should_close() = 0;

    // For now only one window can have imgui callback, multiple imgui context
    // is too messy to maintain.
    // Usually the callback will be set on the main window of the application.
    virtual void
    set_imgui_callback(std::optional<std::function<void()>> callback) = 0;

    virtual ~IWindow() = default;
};
} // namespace ars::render