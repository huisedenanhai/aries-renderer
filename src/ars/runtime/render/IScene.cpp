#include "IScene.h"
#include <ars/runtime/core/misc/Visitor.h>

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

Frustum Perspective::frustum(float w_div_h) const {
    float tan_half_fov_y = std::tan(y_fov * 0.5f);
    float tan_half_fov_x = tan_half_fov_y * w_div_h;

    Frustum f{};
    f.vertices[0] = glm::vec3{-tan_half_fov_x, tan_half_fov_y, -1.0f} * z_near;
    f.vertices[1] = glm::vec3{tan_half_fov_x, tan_half_fov_y, -1.0f} * z_near;
    f.vertices[2] = glm::vec3{tan_half_fov_x, -tan_half_fov_y, -1.0f} * z_near;
    f.vertices[3] = glm::vec3{-tan_half_fov_x, -tan_half_fov_y, -1.0f} * z_near;
    f.vertices[4] = glm::vec3{-tan_half_fov_x, tan_half_fov_y, -1.0f} * z_far;
    f.vertices[5] = glm::vec3{tan_half_fov_x, tan_half_fov_y, -1.0f} * z_far;
    f.vertices[6] = glm::vec3{tan_half_fov_x, -tan_half_fov_y, -1.0f} * z_far;
    f.vertices[7] = glm::vec3{-tan_half_fov_x, -tan_half_fov_y, -1.0f} * z_far;

    f.update_planes_by_vertices();

    return f;
}

glm::mat4 Orthographic::projection_matrix(float w_div_h) const {
    auto x_mag = y_mag * w_div_h;
    return ortho_reverse_z(-x_mag, x_mag, -y_mag, y_mag, z_near, z_far);
}

Frustum Orthographic::frustum(float w_div_h) const {
    auto x_mag = y_mag * w_div_h;
    Frustum f{};
    f.vertices[0] = {-x_mag, y_mag, -z_near};
    f.vertices[1] = {x_mag, y_mag, -z_near};
    f.vertices[2] = {x_mag, -y_mag, -z_near};
    f.vertices[3] = {-x_mag, -y_mag, -z_near};
    f.vertices[4] = {-x_mag, y_mag, -z_far};
    f.vertices[5] = {x_mag, y_mag, -z_far};
    f.vertices[6] = {x_mag, -y_mag, -z_far};
    f.vertices[7] = {-x_mag, -y_mag, -z_far};

    f.update_planes_by_vertices();

    return f;
}

glm::mat4 CameraData::projection_matrix(float w_div_h) const {
    return ars::visit(
        [w_div_h](auto &&d) { return d.projection_matrix(w_div_h); }, *this);
}

Frustum CameraData::frustum(float w_div_h) const {
    return ars::visit([w_div_h](auto &&d) { return d.frustum(w_div_h); },
                      *this);
}

float CameraData::z_far() const {
    return ars::visit([](auto &&d) { return d.z_far; }, *this);
}

float CameraData::z_near() const {
    return ars::visit([](auto &&d) { return d.z_near; }, *this);
}

glm::mat4 IView::projection_matrix() {
    auto w_div_h = size().w_div_h();
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

void IView::render() {
    RenderOptions opts{};
    render(opts);
}

void IOverlay::draw_wire_box(const math::XformTRS<float> &xform,
                             const glm::vec3 &center,
                             const glm::vec3 &extent,
                             const glm::vec4 &color) {

    auto m = xform.matrix();
    glm::vec3 points_norm[8] = {
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, -0.5f},
    };

    auto get_point = [&](int index) {
        return glm::vec3(
            math::transform_position(m, center + points_norm[index] * extent));
    };

    for (int i = 0; i < 4; i++) {
        auto a0 = get_point(i);
        auto b0 = get_point((i + 1) % 4);
        auto a1 = get_point(i + 4);
        auto b1 = get_point((i + 1) % 4 + 4);
        draw_line(a0, b0, color);
        draw_line(a1, b1, color);
        draw_line(a0, a1, color);
        draw_line(b0, b1, color);
    }
}

Frustum transform_frustum(const glm::mat4 &mat, const Frustum &frustum) {
    Frustum f = frustum;
    for (int i = 0; i < std::size(frustum.planes); i++) {
        f.planes[i] = math::transform_plane(mat, frustum.planes[i]);
    }
    for (int i = 0; i < std::size(frustum.vertices); i++) {
        f.vertices[i] = math::transform_position(mat, frustum.vertices[i]);
    }
    return f;
}

float depth01_to_linear_z(const glm::mat4 &I_P, float depth01) {
    auto pos = math::transform_position(I_P, {0.0f, 0.0f, depth01});
    return -pos.z / pos.w;
}

float linear_z_to_depth01(const glm::mat4 &P, float linear_z) {
    auto pos = math::transform_position(P, {0.0f, 0.0f, -linear_z});
    return pos.z / pos.w;
}

bool Frustum::culled(const math::AABB<float> &aabb) const {
    // Conservative culling, some aabb that not intersect with frustum may not
    // be culled. But those culled are definitely out of view.
    for (auto &p : planes) {
        auto culled_point = [&](const glm::vec3 &v) {
            return glm::dot(p, glm::vec4(aabb.lerp(v), 1.0f)) > 0;
        };

        bool culled = true;
        culled = culled && culled_point({0.0f, 0.0f, 0.0f});
        culled = culled && culled_point({0.0f, 0.0f, 1.0f});
        culled = culled && culled_point({0.0f, 1.0f, 0.0f});
        culled = culled && culled_point({0.0f, 1.0f, 1.0f});
        culled = culled && culled_point({1.0f, 0.0f, 0.0f});
        culled = culled && culled_point({1.0f, 0.0f, 1.0f});
        culled = culled && culled_point({1.0f, 1.0f, 0.0f});
        culled = culled && culled_point({1.0f, 1.0f, 1.0f});
        if (culled) {
            return true;
        }
    }
    return false;
}

std::array<uint32_t, 24> Frustum::edges() {
    return {
        0, 1, 1, 2, 2, 3, 3, 0, // near plane
        0, 4, 1, 5, 2, 6, 3, 7, // connections
        4, 5, 5, 6, 6, 7, 7, 4, // far plane
    };
}

Frustum Frustum::crop(const math::AABB<float> &proportion) const {
    glm::vec3 coords[8] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
    };

    Frustum f{};
    for (int i = 0; i < 8; i++) {
        f.vertices[i] = lerp(proportion.lerp(coords[i]));
    }
    f.update_planes_by_vertices();

    return f;
}

void Frustum::update_planes_by_vertices() {
    auto form_plane = [&](uint32_t v0, uint32_t v1, uint32_t v2) {
        return math::calculate_plane_from_points(
            vertices[v0], vertices[v1], vertices[v2]);
    };

    // +X
    planes[0] = form_plane(1, 2, 5);
    // -X
    planes[1] = form_plane(3, 0, 4);
    // +Y
    planes[2] = form_plane(0, 1, 5);
    // -Y
    planes[3] = form_plane(2, 3, 7);
    // +Z
    planes[4] = form_plane(2, 1, 0);
    // -Z
    planes[5] = form_plane(7, 4, 5);
}

glm::vec3 Frustum::lerp(const glm::vec3 &p) const {
    auto p_near = glm::mix(glm::mix(vertices[0], vertices[1], p.x),
                           glm::mix(vertices[3], vertices[2], p.x),
                           p.y);
    auto p_far = glm::mix(glm::mix(vertices[4], vertices[5], p.x),
                          glm::mix(vertices[7], vertices[6], p.x),
                          p.y);

    return glm::mix(p_near, p_far, p.z);
}

math::AABB<float> Frustum::aabb() const {
    return math::AABB<float>::from_points(std::begin(vertices),
                                          std::end(vertices));
}
} // namespace ars::render
