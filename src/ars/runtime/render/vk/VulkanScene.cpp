#include "VulkanScene.h"

namespace ars::render {
std::unique_ptr<IRenderObject> VulkanScene::create_renderer() {
    return std::make_unique<VulkanRenderObject>();
}

std::unique_ptr<IDirectionalLight> VulkanScene::create_directional_light() {
    return std::make_unique<VulkanDirectionalLight>();
}

std::unique_ptr<IView> VulkanScene::create_view() {
    return std::make_unique<VulkanView>();
}

IScene *VulkanView::get_scene() {
    return nullptr;
}

void VulkanView::render() {}

ITexture *VulkanView::get_color_texture() {
    return nullptr;
}

math::AffineTransform<float> VulkanDirectionalLight::get_transform() {
    return math::AffineTransform<float>();
}

void VulkanDirectionalLight::set_transform(
    const math::AffineTransform<float> &transform) {}

IScene *VulkanDirectionalLight::get_scene() {
    return nullptr;
}

math::AffineTransform<float> VulkanRenderObject::get_transform() {
    return math::AffineTransform<float>();
}

void VulkanRenderObject::set_transform(
    const math::AffineTransform<float> &transform) {}

IScene *VulkanRenderObject::get_scene() {
    return nullptr;
}

IMesh *VulkanRenderObject::get_mesh() {
    return nullptr;
}

void VulkanRenderObject::set_mesh(IMesh *mesh) {}

IMaterial *VulkanRenderObject::get_material() {
    return nullptr;
}

void VulkanRenderObject::set_material(IMaterial *mesh) {}
} // namespace ars::render
