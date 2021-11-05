#pragma once

#include "Vulkan.h"

namespace ars::render::vk {
class Texture;
class Context;

class Lut {
  public:
    explicit Lut(Context *context);
    ~Lut();

    Handle<Texture> brdf_lut() const;

  private:
    void init_brdf_lut();

    Context *_context = nullptr;
    Handle<Texture> _brdf_lut{};
};
} // namespace ars::render::vk