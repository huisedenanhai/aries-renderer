#pragma once

#include <ars/runtime/engine/Entity.h>

namespace ars::editor {
void save_entity(engine::Entity *entity, const std::filesystem::path &path);
void load_entity(engine::Entity *entity, const std::filesystem::path &path);
} // namespace ars::editor