#include "RenderSystemComponent.h"

namespace ars::render {
void RenderSystemComponent::register_component() {
    scene::register_component<RenderSystemComponent>(
        "ars::render::RenderSystemComponent")(
        rttr::metadata(scene::ComponentMeta::SystemComponent, true));
}

ARS_REGISTER_COMPONENT(RenderSystemComponent);
} // namespace ars::render