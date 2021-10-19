#pragma once

#include "Common.h"
#include "IMaterial.h"
#include <memory>
#include <optional>
#include <string>

namespace ars::render {
class IWindow;
class ITexture;
class IScene;
class IMesh;

struct TextureInfo;
struct MeshInfo;
struct WindowInfo;

enum class Backend { Vulkan };

struct ApplicationInfo {
    Backend backend = Backend::Vulkan;
    std::string app_name{};

    struct Version {
        int major = 0;
        int minor = 0;
        int patch = 0;
    };

    Version version{};

    bool enable_presentation = true;
    bool enable_validation = false;
};

void init_render_backend(const ApplicationInfo &info);
void destroy_render_backend();

// The root of the renderer. The factory of render resources.
// All resources should be destroyed before context is released.
class IContext {
  public:
    virtual ~IContext() = default;

    // If window_info == nullptr, we assume the context is not used for
    // presentation, and later calls to create_window always returns nullptr.
    static std::pair<std::unique_ptr<IContext>, std::unique_ptr<IWindow>>
    create(WindowInfo *window_info);

    // This method returns nullptr if window creation fails
    virtual std::unique_ptr<IWindow> create_window(const WindowInfo &info) = 0;
    virtual std::unique_ptr<IScene> create_scene() = 0;

    // Texture/Mesh/IMaterial generally requires multiple ownership. return a
    // shared_ptr by default.
    std::shared_ptr<ITexture> create_texture(const TextureInfo &info);
    std::shared_ptr<ITexture>
    create_texture_2d(Format format,
                      uint32_t width,
                      uint32_t height,
                      uint32_t mip_levels = MAX_MIP_LEVELS);

    virtual std::shared_ptr<IMesh> create_mesh(const MeshInfo &info) = 0;
    // Return the prototype for error color material if the material type is not
    // supported
    virtual IMaterialPrototype *material_prototype(MaterialType type) = 0;

    // Call this method when a frame begins. If this method returns false,
    // the backend refuse to begin a new frame and no render work should be
    // submitted. If this method returns true, a call to end_frame() is required
    // to denote the end of the frame.
    //
    // begin_frame() and end_frame() are hints to backend for synchronization.
    //
    // e.g.
    // if (ctx->begin_frame()) {
    //   // submit your render works here
    //   ctx->end_frame();
    // }
    virtual bool begin_frame() = 0;
    virtual void end_frame() = 0;

  protected:
    virtual std::shared_ptr<ITexture>
    create_texture_impl(const TextureInfo &info) = 0;
};

} // namespace ars::render