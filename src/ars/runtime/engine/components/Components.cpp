#include "Components.h"
#include "RenderSystem.h"

namespace ars::engine {
void register_components() {
    MeshRenderer::register_component();
    PointLight::register_component();
    DirectionalLight::register_component();
    Camera::register_component();
}
} // namespace ars::engine