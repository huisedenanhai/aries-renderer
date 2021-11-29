#pragma once

#include <ars/runtime/core/Res.h>

namespace ars::engine {
struct ResRef {
    std::string path;
    std::shared_ptr<IRes> res;
    std::function<void(const std::shared_ptr<IRes> &)> on_change = [](auto) {};
};
} // namespace ars::engine