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
} // namespace ars::render
