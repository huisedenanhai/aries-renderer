#pragma once

#include "Common.h"
#include <memory>
#include <optional>
#include <string>

namespace ars::render {
class IWindow;
class ITexture;
class IScene;
class IMesh;
class ISkin;
class IPanoramaSky;
class IPhysicalSky;
class IMaterial;

struct TextureInfo;
struct MeshInfo;
struct SkinInfo;
struct WindowInfo;
struct MaterialInfo;

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
    bool enable_profiler = false;
};

enum class DefaultTexture : uint32_t {
    White,
    Zero,
    Normal,
    WhiteCubeMap,
    Count
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
    // If the window_info.logical_size is {0, 0}, a window with the size of the
    // monitor will be created
    virtual std::unique_ptr<IWindow> create_window(const WindowInfo &info) = 0;
    virtual std::unique_ptr<IScene> create_scene() = 0;

    // Texture/Mesh/IMaterial generally requires multiple ownership. return a
    // shared_ptr by default.
    std::shared_ptr<ITexture> create_texture(const TextureInfo &info);
    virtual std::shared_ptr<ITexture> default_texture(DefaultTexture tex) = 0;

    virtual std::shared_ptr<IMesh> create_mesh(const MeshInfo &info) = 0;
    virtual std::shared_ptr<ISkin> create_skin(const SkinInfo &info) = 0;
    virtual std::shared_ptr<IMaterial>
    create_material(const MaterialInfo &info) = 0;

    virtual std::shared_ptr<IPanoramaSky> create_panorama_sky() = 0;
    virtual std::shared_ptr<IPhysicalSky> create_physical_sky() = 0;

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