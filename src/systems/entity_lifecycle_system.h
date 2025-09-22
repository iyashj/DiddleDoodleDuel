#ifndef DIDDLEDOODLEDUEL_ENTITY_LIFECYCLE_SYSTEM_H
#define DIDDLEDOODLEDUEL_ENTITY_LIFECYCLE_SYSTEM_H
#include <entt/entity/registry.hpp>
#include "core/scene_state.h"
#include "components/scene_entity.h"
#include <vector>

struct EntityLifecycleSystem {

    static void cleanupSceneEntities(entt::registry& registry, const SceneType scene) {
        std::vector<entt::entity> toDestroy;
        toDestroy.reserve(32);

        for (const auto view = registry.view<const SceneEntity>(); const auto entity : view) {
            if (const auto& [belongsToScene, persistent] = view.get<const SceneEntity>(entity);
                belongsToScene == scene && !persistent) {
                toDestroy.push_back(entity);
            }
        }

        registry.destroy(toDestroy.begin(), toDestroy.end());
    }
    
    // Clean up ALL entities (for destructor/reset scenarios)
    static void cleanupAllEntities(entt::registry& registry) {
        registry.clear();
    }

    static void tagEntityWithScene(entt::registry& registry,
                                 const entt::entity entity,
                                 const SceneType scene,
                                 const bool persistent = false) {
        registry.emplace_or_replace<SceneEntity>(entity, SceneEntity {.belongsToScene=scene, .persistent=persistent} );
    }
    
    // Process lifecycle events (deaths, spawns, etc.)
    static void processLifecycle([[maybe_unused]] entt::registry& registry) {
        // Framework ready for:
        // - Entities marked for death
        // - Delayed spawning
        // - Lifecycle state transitions
    }
};

#endif // DIDDLEDOODLEDUEL_ENTITY_LIFECYCLE_SYSTEM_H
