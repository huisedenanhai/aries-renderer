#include "IMesh.h"
#include <rttr/registration>

namespace ars::render {
IMesh::IMesh(const MeshInfo &info) : _info(info) {}

size_t IMesh::vertex_capacity() const {
    return _info.vertex_capacity;
}

size_t IMesh::triangle_capacity() const {
    return _info.triangle_capacity;
}

void IMesh::register_type() {
    rttr::registration::class_<IMesh>("ars::render::IMesh");
}

bool IMesh::skinned() const {
    return _info.skinned;
}
} // namespace ars::render
