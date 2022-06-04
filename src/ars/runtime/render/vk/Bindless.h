#pragma once

#include <vector>

namespace ars::render::vk {
class Texture;

class BindlessResources {
  public:
    int32_t make_resident(Texture *tex);
    void make_none_resident(Texture *tex);

  private:
    std::vector<Texture *> textures_2d{};
};
} // namespace ars::render::vk