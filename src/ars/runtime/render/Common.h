#pragma once

#include <cstdint>
#include <limits>

namespace ars::render {
enum class Format { R8G8B8A8Srgb };
constexpr uint32_t MAX_MIP_LEVELS = std::numeric_limits<uint32_t>::max();
} // namespace ars::render