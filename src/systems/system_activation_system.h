#ifndef DIDDLEDOODLEDUEL_SYSTEM_ACTIVATION_H
#define DIDDLEDOODLEDUEL_SYSTEM_ACTIVATION_H
#include <entt/entity/registry.hpp>
#include <string>
#include "core/scene_state.h"

struct SystemsActivationSystem {
    static void processActivations(entt::registry& registry) {
        registry.ctx().get<SceneState>().updateActiveSystems();
    }

    static bool shouldSystemRun(const entt::registry& registry, const std::string& systemName) {
        const auto& state = registry.ctx().get<SceneState>();
        return state.activeSystems.contains(systemName);
    }
};

#endif // DIDDLEDOODLEDUEL_SYSTEM_ACTIVATION_H
