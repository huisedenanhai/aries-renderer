#include "Spawn.h"
#include "Entity.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Serde.h>

namespace ars::engine {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_DEFINITION(EntityData,
                                              name,
                                              xform,
                                              components)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_DEFINITION(SpawnData,
                                              entities,
                                              hierarchies,
                                              root)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_DEFINITION(SpawnData::Hierarchy, children)

EntityData EntityData::from(Entity *entity) {
    if (entity == nullptr) {
        return {};
    }
    EntityData data{};
    data.name = entity->name();
    data.xform = entity->local_xform();

    auto &comps_js = data.components;
    for (auto comp : entity->components()) {
        nlohmann::json c_js = {
            {"type", comp->type().get_name().to_string()},
            {"value", comp->serialize()},
        };

        comps_js.emplace_back(std::move(c_js));
    }

    return data;
}

void EntityData::to(Entity *entity, uint32_t modify_mask) const {
    if (entity == nullptr) {
        return;
    }
    if (modify_mask & MODIFY_MASK_NAME_BIT) {
        entity->set_name(name);
    }
    if (modify_mask & MODIFY_MASK_XFORM_BIT) {
        entity->set_local_xform(xform);
    }
    if (modify_mask & MODIFY_MASK_COMPONENTS_BIT) {
        for (auto &c_js : components) {
            auto ty_name = c_js["type"].get<std::string>();
            auto ty = rttr::type::get_by_name(ty_name);
            if (entity->component(ty) != nullptr) {
                entity->remove_component(ty);
            }
            auto comp = entity->add_component(ty);
            comp->deserialize(c_js["value"]);
        }
    }
}

SpawnData SpawnData::from(Entity *root) {
    std::vector<Entity *> entities{};
    std::map<Entity *, int> indices{};
    root->visit_preorder([&](Entity *entity) {
        indices[entity] = static_cast<int>(entities.size());
        entities.push_back(entity);
    });

    SpawnData data{};
    data.entities.reserve(entities.size());
    data.hierarchies.reserve(entities.size());
    for (auto entity : entities) {
        Hierarchy hierarchy{};
        hierarchy.children.reserve(entity->child_count());
        for (int i = 0; i < entity->child_count(); i++) {
            auto child = entity->child(i);
            hierarchy.children.push_back(indices[child]);
        }
        data.hierarchies.emplace_back(std::move(hierarchy));
        data.entities.emplace_back(EntityData::from(entity));
    }

    data.root = indices[root];
    return data;
}

void SpawnData::to(Entity *root_entity) const {
    if (root_entity == nullptr) {
        return;
    }

    auto entity_count = entities.size();
    if (root >= entity_count) {
        ARS_LOG_ERROR("Invalid SpawnData: root index >= entity count");
        return;
    }
    if (entities.size() != hierarchies.size()) {
        ARS_LOG_ERROR(
            "Invalid SpawnData: entities count and hierarchies count mismatch");
        return;
    }
    std::vector<Entity *> spawned{};
    spawned.reserve(entity_count);
    for (int i = 0; i < entity_count; i++) {
        spawned.push_back(i == root ? root_entity
                                    : root_entity->scene()->create_entity());
    }
    for (int i = 0; i < entity_count; i++) {
        for (auto child_index : hierarchies[i].children) {
            spawned[child_index]->set_parent(spawned[i]);
        }
    }
    for (int i = 0; i < entity_count; i++) {
        auto e = spawned[i];
        if (i != root) {
            entities[i].to(e);
        } else {
            entities[i].to(e, EntityData::MODIFY_MASK_COMPONENTS_BIT);
        }
    }
}
} // namespace ars::engine