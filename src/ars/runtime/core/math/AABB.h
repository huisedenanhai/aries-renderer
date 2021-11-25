#pragma once

#include <glm/glm.hpp>

namespace ars::math {
template <typename T> struct AABB {
    using Vec3 = glm::vec<3, T>;
    AABB() = default;
    explicit AABB(const Vec3 &p) : min(p), max(p) {}
    AABB(const Vec3 &min, const Vec3 &max) : min(min), max(max) {}

    bool valid() const {
        for (int i = 0; i < 3; i++) {
            if (min[i] > max[i]) {
                return false;
            }
        }
        return true;
    }

    void regulate() {
        for (int i = 0; i < 3; i++) {
            if (min[i] > max[i]) {
                std::swap(min[i], max[i]);
            }
        }
    }

    Vec3 center() const {
        return static_cast<T>(0.5) * (min + max);
    }

    Vec3 extent() const {
        return max - min;
    }

    // Inclusive
    bool contains(const Vec3 &p) const {
        for (int i = 0; i < 3; i++) {
            if (p[i] < min[i] || p[i] > max[i]) {
                return false;
            }
        }
        return true;
    }

    void extend_point(const Vec3 &p) {
        for (int i = 0; i < 3; i++) {
            min[i] = std::min(min[i], p[i]);
            max[i] = std::max(max[i], p[i]);
        }
    }

    void extend_aabb(const AABB &rhs) {
        extend_point(rhs.min);
        extend_point(rhs.max);
    }

    template <typename Iter> static AABB from_points(Iter beg, Iter end) {
        if (beg == end) {
            return {};
        }
        AABB aabb(*beg);
        beg++;
        for (; beg != end; beg++) {
            aabb.extend_point(*beg);
        }
        return aabb;
    }

    Vec3 lerp(const Vec3 &v) const {
        return min + extent() * v;
    }

    // One should ensure: for each component c, min[c] <= max[c]
    Vec3 min{};
    Vec3 max{};
};

template <typename T>
AABB<T> transform_aabb(const glm::mat<4, 4, T> &mat, const AABB<T> &aabb) {
    using Vec3 = typename AABB<T>::Vec3;

    AABB<T> res{};
    for (int i = 0; i < 8; i++) {
        Vec3 local = {
            (i & 1) ? 1.0f : 0.0f,
            (i & 2) ? 1.0f : 0.0f,
            (i & 4) ? 1.0f : 0.0f,
        };
        auto p = aabb.lerp(local);
        auto p_ws = transform_position(mat, p);

        if (i == 0) {
            res = AABB<T>(p_ws);
        } else {
            res.extend_point(p_ws);
        }
    }
    return res;
}
} // namespace ars::math