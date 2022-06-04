#include "Bindless.h"
#include "Texture.h"

namespace ars::render::vk {
int32_t BindlessResources::make_resident(Texture *tex) {
    if (tex->info().view_type == VK_IMAGE_VIEW_TYPE_2D) {
        textures_2d.push_back(tex);
        return static_cast<int32_t>(textures_2d.size() - 1);
    }
    return -1;
}

void BindlessResources::make_none_resident(Texture *tex) {
    if (tex->bindless_id() < 0) {
        return;
    }

    if (tex->info().view_type == VK_IMAGE_VIEW_TYPE_2D) {
        textures_2d[tex->bindless_id()] = nullptr;
    }
}

BindlessResources::BindlessResources(Context *context) : _context(context) {}
} // namespace ars::render::vk