#ifndef DIDDLEDOODLEDUEL_USERNAME_RENDER_H
#define DIDDLEDOODLEDUEL_USERNAME_RENDER_H

#include "components/position.h"
#include "components/username.h"
#include "components/renderable.h"
#include "components/scene_entity.h"
#include "systems/scene_transition_system.h"
#include "rendering/irenderer.h"
#include <entt/entity/registry.hpp>
#include <raylib.h>

struct UsernameRenderSystem {

    explicit UsernameRenderSystem(entt::registry& registry, engine::IRenderer& renderer)
        : registry(registry), renderer(renderer) {}

    void render() const {
        const auto currentScene = SceneTransitionSystem::getCurrentScene(registry);
        
        for (const auto view = registry.view<const Position, const Username, const Renderable, const SceneEntity>();
             const auto entity : view) {
            const auto& [position] = view.get<const Position>(entity);
            const auto& [text] = view.get<const Username>(entity);

            // Only render entities that belong to the current scene
            if (const auto& [belongsToScene, persistent] = view.get<const SceneEntity>(entity);
                belongsToScene != currentScene) {
                continue;
            }

            // Calculate position above the player
            Vector2 textPosition = {
                position.x - (text.length() * 4.0f), // Rough centering
                position.y - 30.0f  // Above the player
            };

            // Draw the username
            renderer.drawText(text, textPosition, 18, BLACK);
        }
    }

private:
    entt::registry& registry;
    engine::IRenderer& renderer;
};

#endif // DIDDLEDOODLEDUEL_USERNAME_RENDER_H