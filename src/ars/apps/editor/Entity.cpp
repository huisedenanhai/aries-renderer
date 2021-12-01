#include "Entity.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/engine/Spawn.h>
#include <fstream>

namespace ars::editor {
void save_entity(engine::Entity *entity, const std::filesystem::path &path) {
    nlohmann::json js = engine::SpawnData::from(entity);
    std::ofstream os(path);
    os << std::setw(2) << js << std::endl;
    os.close();
    ARS_LOG_INFO("Save entity {} to {}", entity->name(), path.string());
}

void load_entity(engine::Entity *entity, const std::filesystem::path &path) {
    nlohmann::json js{};
    std::ifstream is(path);
    is >> js;
    is.close();
    engine::SpawnData spawn = js;
    spawn.to(entity);
}
} // namespace ars::editor