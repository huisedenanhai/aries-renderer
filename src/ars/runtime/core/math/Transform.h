#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ars::math {
template <typename T> struct XformTRS {
  public:
    using Mat4 = glm::mat<4, 4, T>;
    using Mat3 = glm::mat<3, 3, T>;
    using Vec3 = glm::vec<3, T>;
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

    friend XformTRS operator*(const XformTRS &lhs, const XformTRS &rhs) {
        return XformTRS(lhs.matrix() * rhs.matrix());
    }

    static XformTRS from_translation(const Vec3 &t) {
        XformTRS xform{};
        xform.set_translation(t);
        return xform;
    }

  private:
    Vec3 _translation{0, 0, 0};
    Quat _rotation{1, 0, 0, 0};
    Vec3 _scale{1, 1, 1};
};
} // namespace ars::math