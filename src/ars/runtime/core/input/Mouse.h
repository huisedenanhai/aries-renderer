#pragma once

#include <glm/glm.hpp>

namespace ars::input {
enum class MouseButton { Left, Middle, Right };

class IMouse {
  public:
    virtual ~IMouse() = default;

    virtual bool is_holding(MouseButton button) = 0;
    virtual bool is_pressed(MouseButton button) = 0;
    virtual bool is_released(MouseButton button) = 0;

    // Relative to the window
    virtual glm::dvec2 cursor_position() = 0;
    virtual glm::dvec2 scroll_delta() = 0;
};
} // namespace ars::input