#include "Model.h"
#include <ars/runtime/core/misc/Visitor.h>

namespace ars::render {
Model::Camera::Type Model::Camera::type() const {
    if (std::holds_alternative<PerspectiveData>(data)) {
        return Model::Camera::Perspective;
    }
    return Model::Camera::Orthographic;
}

glm::mat4 Model::Camera::projection_matrix(float aspect) const {
    return std::visit(make_visitor(
                          [](const PerspectiveData &pers) {
                              return glm::identity<glm::mat4>();
                          },
                          [](const OrthographicData &ortho) {
                              return glm::identity<glm::mat4>();
                          }),
                      data);
}

Model load_gltf(const std::filesystem::path &path) {
    return Model{};
}
} // namespace ars::render