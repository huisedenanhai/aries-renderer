#pragma once

#include "Entity.h"

namespace ars::engine::editor {
void hierarchy_inspector(Scene *scene, Entity *&current_selected);
void entity_inspector(Entity *entity);
} // namespace ars::engine::editor