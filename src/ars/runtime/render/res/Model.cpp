#include "Model.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>
#include <sstream>
#include <tiny_gltf.h>

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
                              // TODO
                              return glm::identity<glm::mat4>();
                          },
                          [](const OrthographicData &ortho) {
                              // TODO
                              return glm::identity<glm::mat4>();
                          }),
                      data);
}

Model load_gltf(const std::filesystem::path &path) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;

    auto extension = path.extension();
    auto model_path_str = path.string();

    auto gltf_error = [&](const std::string &info) {
        log_error("Failed to load " + model_path_str + ": " + info);
    };

    auto gltf_warn = [&](const std::string &info) {
        log_warn("Loading " + model_path_str + ": " + info);
    };

    bool ret;
    if (extension == ".gltf") {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path_str);
    } else if (extension == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, model_path_str);
    } else {
        std::stringstream ss;
        ss << "Invalid file extension for GLTF: " << extension
           << ", should be .gltf or .glb";
        gltf_error(ss.str());
        return Model{};
    }

    if (!warn.empty()) {
        gltf_warn(warn);
    }

    if (!err.empty()) {
        gltf_error(err);
        return Model{};
    }

    if (!ret) {
        log_error("Failed to parse " + path.string());
        return Model{};
    }

    return Model{};
}
} // namespace ars::render