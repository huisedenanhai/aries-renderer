#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ars::math {
template <typename T> struct XformTRS {
  public:
    using Mat4 = glm::mat<4, 4, T>;
    using Mat3 = glm::mat<3, 3, T>;
    using Vec3 = glm::vec<3, T>;
    using Vec4 = glm::vec<4, T>;
    using Quat = glm::qua<T>;

    XformTRS() = default;

    XformTRS(const Vec3 &t, const Quat &r, const Vec3 &s)
        : _translation(t), _rotation(r), _scale(s) {}

    explicit XformTRS(const Mat4 &m) {
        _translation = Vec3(m[3]);
        auto rs = Mat3(m[0], m[1], m[2]);
        _scale =
            Vec3(glm::length(rs[0]), glm::length(rs[1]), glm::length(rs[2]));
        _rotation = glm::quat_cast(
            Mat3(rs[0] / _scale.x, rs[1] / _scale.y, rs[2] / _scale.z));
    }

    Vec3 translation() const {
        return _translation;
    }

    void set_translation(const Vec3 &t) {
        _translation = t;
    }

    Quat rotation() const {
        return _rotation;
    }

    void set_rotation(const Quat &r) {
        _rotation = r;
    }

    Vec3 scale() const {
        return _scale;
    }

    void set_scale(const Vec3 &s) {
        _scale = s;
    }

    Mat4 matrix() const {
        auto ident = glm::identity<Mat4>();
        return glm::translate(ident, _translation) * glm::mat4_cast(_rotation) *
               glm::scale(ident, _scale);
    }

    Mat4 matrix_no_scale() const {
        auto ident = glm::identity<Mat4>();
        return glm::translate(ident, _translation) * glm::mat4_cast(_rotation);
    }

    friend XformTRS operator*(const XformTRS &lhs, const XformTRS &rhs) {
        return XformTRS(lhs.matrix() * rhs.matrix());
    }

    XformTRS inverse() const {
        return XformTRS(glm::inverse(matrix()));
    }

    static XformTRS from_translation(const Vec3 &t) {
        XformTRS xform{};
        xform.set_translation(t);
        return xform;
    }

    static XformTRS from_rotation(const Quat &r) {
        XformTRS xform{};
        xform.set_rotation(r);
        return xform;
    }

    Vec3 forward() const {
        return _rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    Vec3 right() const {
        return _rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    Vec3 up() const {
        return _rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

  private:
    Vec3 _translation{0, 0, 0};
    Quat _rotation{1, 0, 0, 0};
    Vec3 _scale{1, 1, 1};
};

template <typename T>
glm::vec<4, T> transform_position(const glm::mat<4, 4, T> &mat,
                                  const glm::vec<3, T> &pos) {
    auto p = mat * glm::vec<4, T>(pos, static_cast<T>(1.0));
    return p;
}

template <typename T>
glm::vec<3, T> transform_direction(const glm::mat<4, 4, T> &mat,
                                   const glm::vec<3, T> &dir) {
    auto p = mat * glm::vec<4, T>(dir, static_cast<T>(0.0));
    return {p.x, p.y, p.z};
}

template <typename T>
glm::vec<4, T> transform_plane(const glm::mat<4, 4, T> &mat,
                               const glm::vec<4, T> &plane) {
    return plane * glm::inverse(mat);
}

// Form a plane based on 3 vertices. Vertices are right hand ordered.
template <typename T>
glm::vec<4, T> calculate_plane_from_points(const glm::vec<3, T> &v0,
                                           const glm::vec<3, T> &v1,
                                           const glm::vec<3, T> &v2) {
    auto n = glm::normalize(glm::cross(v1 - v0, v2 - v1));
    auto c = -glm::dot(n, v0);
    return {n, c};
}

} // namespace ars::math