#include "IWindow.h"

namespace ars::render {
glm::dvec2 convert_window_coord(const glm::dvec2 &pos,
                                const Extent2D &from,
                                const Extent2D &to) {
    return {
        pos.x * to.width / from.width,
        pos.y * to.height / from.height,
    };
}

glm::dvec2 IWindow::from_logical_to_physical(const glm::dvec2 &pos) {
    return convert_window_coord(pos, logical_size(), physical_size());
}

glm::dvec2 IWindow::from_physical_to_logical(const glm::dvec2 &pos) {
    return convert_window_coord(pos, physical_size(), logical_size());
}
} // namespace ars::render
