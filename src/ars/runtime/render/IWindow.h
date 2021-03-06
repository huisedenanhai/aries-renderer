#pragma once

#include "Common.h"
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>

namespace ars::input {
class IKeyBoard;
class IMouse;
} // namespace ars::input

namespace ars::render {
class ITexture;

struct WindowInfo {
    std::string title{};
    Extent2D logical_size{0, 0};
};

enum class CursorMode {
    Normal,
    Hidden,
    HiddenCaptured,
};

glm::dvec2 convert_window_coord(const glm::dvec2 &pos,
                                const Extent2D &from,
                                const Extent2D &to);

class IWindow {
  public:
    // Blit a 2d texture to screen and swap buffer.
    virtual void present(ITexture *texture) = 0;

    virtual Extent2D physical_size() = 0;
    virtual Extent2D logical_size() = 0;

    glm::dvec2 from_logical_to_physical(const glm::dvec2 &pos);
    glm::dvec2 from_physical_to_logical(const glm::dvec2 &pos);

    virtual bool should_close() = 0;

    // For now only one window can have imgui callback, multiple imgui context
    // is too messy to maintain.
    // Usually the callback will be set on the main window of the application.
    virtual void
    set_imgui_callback(std::optional<std::function<void()>> callback) = 0;

    // Should always return a valid pointer for input devices.
    // A dummy device should be returned if the physical device is not present.
    virtual input::IKeyBoard *keyboard() = 0;
    virtual input::IMouse *mouse() = 0;

    virtual CursorMode cursor_mode() = 0;
    virtual void set_cursor_mode(CursorMode mode) = 0;

    virtual ~IWindow() = default;
};
} // namespace ars::render