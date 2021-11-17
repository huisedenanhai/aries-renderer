#include "IScene.h"

namespace ars::render {
namespace {
glm::mat4
perspective_reverse_z(float y_fov, float w_div_h, float z_near, float z_far) {
    float tan_half_fovy = std::tan(y_fov * 0.5f);
    float one = 1.0f;
    return {
        {one / (w_div_h * tan_half_fovy), 0, 0, 0},
        {0, -one / tan_half_fovy, 0, 0},
        {0, 0, z_near / (z_far - z_near), -one},
        {0, 0, (z_far * z_near) / (z_far - z_near), 0},
    };
}

glm::mat4
perspective_reverse_z_infinite_z_far(float y_fov, float w_div_h, float z_near) {
    float tan_half_fovy = std::tan(y_fov * 0.5f);
    float one = 1.0f;
    return {{one / (w_div_h * tan_half_fovy), 0, 0, 0},
            {0, -one / tan_half_fovy, 0, 0},
            {0, 0, 0, -one},
            {0, 0, z_near, 0}};
}

glm::mat4 ortho_reverse_z(float left,
                          float right,
                          float bottom,
                          float top,
                          float z_near,
                          float z_far) {
    float one = 1.0f;
    float two = 2.0f;
    return {{two / (right - left), 0, 0, 0},
            {0, two / (bottom - top), 0, 0},
            {0, 0, one / (z_far - z_near), 0},
            {(left + right) / (left - right),
             (top + bottom) / (top - bottom),
             z_far / (z_far - z_near),
             one}};
}
} // namespace

// Map scissor NDC region range [-1, 1]
glm::mat4 projection_scissor_correction(uint32_t x,
                                        uint32_t y,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t image_width,
                                        uint32_t image_height) {
    float ax = static_cast<float>(image_width) / static_cast<float>(width);
    float ay = static_cast<float>(image_height) / static_cast<float>(height);
    float bx =
        -1.0f + ax - 2.0f * static_cast<float>(x) / static_cast<float>(width);
    float by =
        -1.0f + ay - 2.0f * static_cast<float>(y) / static_cast<float>(height);
    return {
        // clang-format off
        ax, 0.0f, 0.0f, 0.0f,
        0.0f, ay, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        bx, by, 0.0f, 1.0f,
        // clang-format on
    };
}

glm::mat4 Perspective::projection_matrix(float w_div_h) const {
    if (z_far == 0.0f) {
        return perspective_reverse_z_infinite_z_far(y_fov, w_div_h, z_near);
    }

    return perspective_reverse_z(y_fov, w_div_h, z_near, z_far);
}

glm::mat4 Orthographic::projection_matrix(float w_div_h) const {
    auto x_mag = y_mag * w_div_h;
    return ortho_reverse_z(-x_mag, x_mag, -y_mag, y_mag, z_near, z_far);
}

glm::mat4 CameraData::projection_matrix(float w_div_h) const {
    return std::visit(
        [w_div_h](auto &&d) { return d.projection_matrix(w_div_h); }, *this);
}

float CameraData::z_far() const {
    return std::visit([](auto &&d) { return d.z_far; }, *this);
}

float CameraData::z_near() const {
    return std::visit([](auto &&d) { return d.z_near; }, *this);
}

glm::mat4 IView::projection_matrix() {
    auto w_div_h =
        static_cast<float>(size().width) / static_cast<float>(size().height);
    return camera().projection_matrix(w_div_h);
}

glm::mat4 IView::view_matrix() {
    return glm::inverse(xform().matrix_no_scale());
}

glm::mat4 IView::billboard_MV_matrix(const glm::vec3 &center_ws,
                                     float width,
                                     float height) {
    auto v_matrix = view_matrix();
    auto center_vs = math::transform_position(v_matrix, center_ws);
    return {
        // clang-format off
        width, 0.0, 0.0, 0.0,
        0.0, -height, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        center_vs.x, center_vs.y, center_vs.z, 1.0,
        // clang-format on
    };
}

} // namespace ars::render
