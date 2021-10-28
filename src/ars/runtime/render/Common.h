#pragma once

#include <cstdint>
#include <limits>

namespace ars::render {
enum class Format {
    R8_SRGB,
    R8_UNORM,
    R8G8_SRGB,
    R8G8_UNORM,
    R8G8B8_SRGB,
    R8G8B8_UNORM,
    R8G8B8A8_SRGB,
    R8G8B8A8_UNORM,
    R32G32B32A32_SFLOAT,
    B10G11R11_UFLOAT_PACK32
};

constexpr uint32_t MAX_MIP_LEVELS = std::numeric_limits<uint32_t>::max();

struct Extent2D {
    uint32_t width;
    uint32_t height;
};
} // namespace ars::render