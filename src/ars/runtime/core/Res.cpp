#include "Res.h"
#include <rttr/registration>

namespace ars {
std::string IRes::path() const {
    return _path;
}

void IRes::set_path(const std::string &path) {
    _path = path;
}

void IRes::register_type() {
    rttr::registration::class_<IRes>("ars::IRes");
}
} // namespace ars