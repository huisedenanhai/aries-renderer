#include "Scene.h"

namespace ars::render::vk {
std::unique_ptr<IRenderObject> Scene::create_render_object() {
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

math::XformTRS<float> DirectionalLight::get_xform() {
    return math::XformTRS<float>();
}

void DirectionalLight::set_xform(const math::XformTRS<float> &xform) {}

IScene *DirectionalLight::get_scene() {
    return nullptr;
}

math::XformTRS<float> RenderObject::get_xform() {
    return math::XformTRS<float>();
}

void RenderObject::set_xform(const math::XformTRS<float> &xform) {}

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
