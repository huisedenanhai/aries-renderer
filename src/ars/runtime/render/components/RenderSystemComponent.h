#pragma once

#include <ars/runtime/core/scene/Entity.h>

namespace ars::render {
class RenderSystemComponent : public scene::IComponent {
    ARS_COMPONENT(RenderSystemComponent, scene::IComponent);

  public:
    static void register_component();
};
} // namespace ars::render
