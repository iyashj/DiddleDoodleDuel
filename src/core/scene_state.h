#ifndef DIDDLEDOODLEDUEL_SCENE_STATE_H
#define DIDDLEDOODLEDUEL_SCENE_STATE_H

#include "scene_type.h"
#include <string>
#include <unordered_set>
#include <unordered_map>

struct SceneState {

    SceneType currentScene = SceneType::MainMenu;
    SceneType previousScene = SceneType::MainMenu;
    bool isTransitioning = false;
    float transitionTime = 0.0F;
    std::unordered_set<std::string> activeSystems;

    [[nodiscard]] static const std::unordered_map<SceneType, std::unordered_set<std::string>>& getSceneSystemMap() {
        static const std::unordered_map<SceneType, std::unordered_set<std::string>> map = {
            {
                SceneType::MainMenu,
                {"ImGuiSystem"}
            },
            {
                SceneType::Game,
                {
                    "PaintSystem",
                    "PhysicsMovementSystem",
                    "InputSystem",
                    "UISystem",

                    "PhysicsCollisionSystem",
                    "DebugRenderSystem",
                    "ArrowRenderSystem",
                    "ImGuiSystem"}
            },
            {
                SceneType::NetworkingDemo, {"ImGuiSystem"}
            },
            {
                SceneType::Lobby, {"ImGuiSystem"}
            },
            {
                SceneType::NetworkedGame,
                {
                    "PaintSystem",
                    "PhysicsMovementSystem",
                    "InputSystem",
                    "UISystem",
                    "PhysicsCollisionSystem",
                    "DebugRenderSystem",
                    "ArrowRenderSystem",
                    "UsernameRenderSystem",
                    "ImGuiSystem"}
            }
        };
        return map;
    }

    void updateActiveSystems() {
        const auto& systemMap = getSceneSystemMap();
        if (const auto it = systemMap.find(currentScene); it != systemMap.end()) {
            activeSystems = it->second;
        } else {
            activeSystems.clear();
        }
    }
};

#endif // DIDDLEDOODLEDUEL_SCENE_STATE_H
