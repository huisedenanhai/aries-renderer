#include "Scene.h"

namespace ars::render::vk {
std::unique_ptr<IRenderObject> Scene::create_renderer() {
    return std::make_unique<RenderObject>();
}

std::unique_ptr<IDirectionalLight> Scene::create_directional_light() {
    return std::make_unique<DirectionalLight>();
}

std::unique_ptr<IView> Scene::create_view() {
    return std::make_unique<View>();
}

IScene *View::get_scene() {
    return nullptr;
}

void View::render() {}

ITexture *View::get_color_texture() {
    return nullptr;
}

math::AffineTransform<float> DirectionalLight::get_transform() {
    return math::AffineTransform<float>();
}

void DirectionalLight::set_transform(
    const math::AffineTransform<float> &transform) {}

IScene *DirectionalLight::get_scene() {
    return nullptr;
}

math::AffineTransform<float> RenderObject::get_transform() {
    return math::AffineTransform<float>();
}

void RenderObject::set_transform(
    const math::AffineTransform<float> &transform) {}

IScene *RenderObject::get_scene() {
    return nullptr;
}

IMesh *RenderObject::get_mesh() {
    return nullptr;
}

void RenderObject::set_mesh(IMesh *mesh) {}

IMaterial *RenderObject::get_material() {
    return nullptr;
}

void RenderObject::set_material(IMaterial *mesh) {}
} // namespace ars::render::vk
