#ifndef DIDDLEDOODLEDUEL_SCENE_MANAGER_H
#define DIDDLEDOODLEDUEL_SCENE_MANAGER_H

#include "scene_state.h"
#include "scene_type.h"
#include <entt/entity/registry.hpp>
#include <string>
#include <unordered_set>

struct Scene {
    SceneType type;
};

struct SceneConfig {
    SceneType type;
    std::unordered_set<std::string> activeSystems;
    bool isTransitioning = false;
    float transitionDuration = 0.5f;
    float currentTransitionTime = 0.0f;
};

class SceneManager {
public:

    static entt::entity createScene(entt::registry& registry, SceneType type) {
        const auto sceneEntity = registry.create();
        registry.emplace<Scene>(sceneEntity, Scene{type});
        

        auto& config = registry.ctx().emplace_or_replace<SceneConfig>();
        config.type = type;
        config.activeSystems = getSystemsForScene(type);
        config.isTransitioning = false;
        
        return sceneEntity;
    }
    
    static entt::entity getCurrentSceneEntity(entt::registry& registry) {
        if (auto view = registry.view<Scene>(); view.begin() != view.end()) {
            return *view.begin();
        }

        return createScene(registry, SceneType::MainMenu);
    }
    
    static void transitionToScene(entt::registry& registry, const SceneType newScene) {
        auto& config = registry.ctx().get<SceneConfig>();
        if (config.type == newScene) return;
        
        config.isTransitioning = true;
        config.currentTransitionTime = 0.0f;

        const auto view = registry.view<Scene>();
        registry.destroy(view.begin(), view.end());
        
        createScene(registry, newScene);
    }

    static bool isSystemActive(entt::registry& registry, const std::string& systemName) {
        const auto& config = registry.ctx().get<SceneConfig>();
        return config.activeSystems.contains(systemName);
    }

private:
    static std::unordered_set<std::string> getSystemsForScene(const SceneType scene) {
        const auto& map = SceneState::getSceneSystemMap();
        const auto it = map.find(scene);
        return (it != map.end()) ? it->second : std::unordered_set<std::string>{};
    }
};

#endif // DIDDLEDOODLEDUEL_SCENE_MANAGER_H