#include "Res.h"

namespace ars {
std::string IRes::path() const {
    return _path;
}

void IRes::set_path(const std::string &path) {
    _path = path;
}
} // namespace ars