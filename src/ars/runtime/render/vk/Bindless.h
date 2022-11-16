#pragma once

#include <vector>
#include <cstdint>

namespace ars::render::vk {
class Texture;
class Context;

class BindlessResources {
  public:
    explicit BindlessResources(Context *context);

    int32_t make_resident(Texture *tex);
    void make_none_resident(Texture *tex);

  private:
    Context *_context = nullptr;
    std::vector<Texture *> textures_2d{};
};
} // namespace ars::render::vk
