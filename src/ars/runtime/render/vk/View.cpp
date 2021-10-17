#include "View.h"
#include "Context.h"
#include "Material.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Scene.h"
#include "features/Renderer.h"

namespace ars::render::vk {
namespace {
VkExtent2D translate(const Extent2D &size) {
    return {size.width, size.height};
}
} // namespace

void View::render() {
    auto ctx = context();
    _rt_manager->update(translate(_size));

    ctx->queue()->submit_once(
        [&](CommandBuffer *cmd) { _renderer->render(cmd); });

    update_color_tex_adapter();
}

ITexture *View::get_color_texture() {
    return _color_tex_adapter.get();
}

View::View(Scene *scene, const Extent2D &size) : _scene(scene), _size(size) {
    _rt_manager = std::make_unique<RenderTargetManager>(scene->context());
    _rt_manager->update(translate(size));

    alloc_render_targets();

    _renderer = std::make_unique<Renderer>(this);
}

math::XformTRS<float> View::xform() {
    return _xform;
}

void View::set_xform(const math::XformTRS<float> &xform) {
    _xform = xform;
}

void View::set_camera(const CameraData &camera) {
    _camera = camera;
}

CameraData View::camera() {
    return _camera;
}

Extent2D View::size() {
    return _size;
}

void View::set_size(const Extent2D &size) {
    _size = size;
}

void View::update_color_tex_adapter() {
    auto color_rt = render_target(NamedRT_FinalColor);
    auto rt_info = color_tex_info();
    if (_color_tex_adapter == nullptr) {
        _color_tex_adapter =
            std::make_unique<TextureAdapter>(rt_info, color_rt);
    } else {
        *_color_tex_adapter = TextureAdapter(rt_info, color_rt);
    }
}

TextureInfo View::color_tex_info() const {
    TextureInfo info{};
    info.format = Format::R8G8B8A8_SRGB;
    info.width = _size.width;
    info.height = _size.height;
    info.mip_levels = 1;
    info.mipmap_mode = MipmapMode::Nearest;
    return info;
}

void View::alloc_render_targets() {
    auto alloc_named = [&](NamedRT name) {
        _rt_ids[name] = _rt_manager->alloc(rt_info(name));
    };
    alloc_named(NamedRT_FinalColor);
    alloc_named(NamedRT_Depth);
}

View::~View() {
    auto ctx = context();
    ctx->queue()->flush();

    _renderer.reset();
}

Context *View::context() const {
    return _scene->context();
}

Handle<Texture> View::render_target(NamedRT name) const {
    return _rt_manager->get(_rt_ids[name]);
}

RenderTargetManager *View::rt_manager() const {
    return _rt_manager.get();
}

RenderTargetId View::rt_id(NamedRT name) const {
    return _rt_ids[name];
}

Scene *View::vk_scene() const {
    return _scene;
}

RenderTargetInfo View::rt_info(NamedRT name) const {
    RenderTargetInfo info{};

    switch (name) {
    case NamedRT_GBuffer0:
        break;
    case NamedRT_GBuffer1:
        break;
    case NamedRT_GBuffer2:
        break;
    case NamedRT_GBuffer3:
        break;
    case NamedRT_Depth: {
        auto &tex = info.texture =
            TextureCreateInfo::sampled_2d(VK_FORMAT_D32_SFLOAT, 1, 1, 1);
        tex.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        tex.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;
    }
    case NamedRT_FinalColor: {
        auto &tex = info.texture = translate(color_tex_info());
        tex.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    }
    case NamedRT_Count:
        break;
    }

    return info;
}

} // namespace ars::render::vk