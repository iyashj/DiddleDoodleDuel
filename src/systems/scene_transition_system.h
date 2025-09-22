#ifndef DIDDLEDOODLEDUEL_SCENE_TRANSITION_SYSTEM_H
#define DIDDLEDOODLEDUEL_SCENE_TRANSITION_SYSTEM_H
#include <entt/entity/registry.hpp>
#include "core/scene_state.h"
#include "system_activation_system.h"

struct SceneTransitionSystem {
    static void initializeSceneState(entt::registry& registry) {
        if (!registry.ctx().contains<SceneState>()) {
            auto& state = registry.ctx().emplace<SceneState>();
            state.updateActiveSystems();
        }
    }

    static void requestTransition(entt::registry& registry, const SceneType newScene) {
        if (auto& state = registry.ctx().get<SceneState>(); state.currentScene != newScene) {
            state.previousScene = state.currentScene;
            state.currentScene = newScene;
            state.isTransitioning = true;
            state.transitionTime = 0.0F;

            SystemsActivationSystem::processActivations(registry);
        }
    }

    static void processTransitions(entt::registry& registry, const float deltaTime) {
        if (auto& state = registry.ctx().get<SceneState>(); state.isTransitioning) {
            state.transitionTime += deltaTime;

            if (constexpr float transition_duration = 0.1F;
                state.transitionTime >= transition_duration) {
                state.isTransitioning = false;
                state.transitionTime = 0.0F;
            }
        }
    }

    [[nodiscard]] static SceneType getCurrentScene(const entt::registry& registry) {
        return registry.ctx().get<SceneState>().currentScene;
    }
};

#endif // DIDDLEDOODLEDUEL_SCENE_TRANSITION_SYSTEM_H