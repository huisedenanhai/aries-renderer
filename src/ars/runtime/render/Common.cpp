#include "Common.h"

namespace ars::render {
float Extent2D::w_div_h() const {
    return static_cast<float>(width) / static_cast<float>(height);
}
} // namespace ars::render