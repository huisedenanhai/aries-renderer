#pragma once

#include <ars/runtime/core/Res.h>
#include <ars/runtime/core/math/Transform.h>
#include <nlohmann/json.hpp>
#include <string>

namespace ars::engine {
class Entity;

struct EntityData {
    std::string name{};
    math::XformTRS<float> xform{}; // local to world
    std::vector<nlohmann::json> components{};

    friend void to_json(nlohmann::json &js, const EntityData &v);
    friend void from_json(const nlohmann::json &js, EntityData &v);

    static EntityData from(Entity *entity);

    static constexpr uint32_t MODIFY_MASK_NAME_BIT = 1;
    static constexpr uint32_t MODIFY_MASK_XFORM_BIT = 1 << 1;
    static constexpr uint32_t MODIFY_MASK_COMPONENTS_BIT = 1 << 2;
    static constexpr uint32_t MODIFY_MASK_ALL = 0xFFFFFFFF;

    void to(Entity *entity, uint32_t modify_mask = MODIFY_MASK_ALL) const;
};

struct SpawnData : public IRes {
    std::vector<EntityData> entities{};

    struct Hierarchy {
        std::vector<uint32_t> children{};

        friend void to_json(nlohmann::json &js, const Hierarchy &v);
        friend void from_json(const nlohmann::json &js, Hierarchy &v);
    };

    // should have the same size of entities'
    std::vector<Hierarchy> hierarchies{};
    uint32_t root = 0;

    friend void to_json(nlohmann::json &js, const SpawnData &v);
    friend void from_json(const nlohmann::json &js, SpawnData &v);

    // Data from root and all its descendants
    static SpawnData from(Entity *root);

    void to(Entity *root_entity) const;
};
} // namespace ars::engine