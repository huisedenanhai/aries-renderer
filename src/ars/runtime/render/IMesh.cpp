#include "IMesh.h"

namespace ars::render {
IMesh::IMesh(const MeshInfo &info) : _info(info) {}

size_t IMesh::vertex_capacity() const {
    return _info.vertex_capacity;
}

size_t IMesh::triangle_capacity() const {
    return _info.triangle_capacity;
}

std::string IMesh::res_type() const {
    return RES_TYPE_NAME_MESH;
}
} // namespace ars::render
