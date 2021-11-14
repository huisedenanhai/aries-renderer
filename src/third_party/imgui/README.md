# ImGui

https://github.com/ocornut/imgui

Use the branch docking.

Commit c58fb464113435fdb7d122fde87cef4920b3d2c6

Add following codes to imconfig.h

```c++
namespace ars::render {
class ITexture;
}

#define ImTextureID ars::render::ITexture*
```

# ImGuizmo

https://github.com/huisedenanhai/ImGuizmo

Commit 52f301ca08158f16119e5e53748d83d399c5fe79