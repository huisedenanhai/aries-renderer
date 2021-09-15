#include "IMesh.h"

namespace ars::render {
IMesh::IMesh(const MeshInfo &info) : _info(info) {}

size_t IMesh::vertex_capacity() const {
    return _info.vertex_capacity;
}

size_t IMesh::index_capacity() const {
    return _info.index_capacity;
}
} // namespace ars::render
