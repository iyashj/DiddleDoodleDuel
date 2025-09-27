#ifndef DIDDLEDOODLEDUEL_ARROW_RENDER_H
#define DIDDLEDOODLEDUEL_ARROW_RENDER_H
#include "components/position.h"
#include "components/renderable.h"
#include "components/velocity.h"
#include "components/scene_entity.h"
#include "systems/scene_transition_system.h"
#include "rendering/irenderer.h"
#include "resources/resource_manager.h"
#include <entt/entity/registry.hpp>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

struct ArrowRenderSystem {
    entt::registry& registry;
    engine::IRenderer& renderer;
    Texture2D arrowTexture{};

    explicit ArrowRenderSystem(entt::registry& registry, engine::IRenderer& renderer)
        : registry(registry), renderer(renderer) {
        
        if (const auto result = engine::resources::loadTexture("textures/arrowFacingUp.png");
            result.has_value()) {
            this->arrowTexture = result.value();
        }
    }

    void render() const {
        // Don't render if texture failed to load
        if (arrowTexture.id == 0) {
            return;
        }

        const auto currentScene = SceneTransitionSystem::getCurrentScene(registry);
        
        for (const auto view = registry.view<const Position, const Velocity, const Renderable, const SceneEntity>();
             const auto entity : view) {
            const auto& [position] = view.get<const Position>(entity);
            const auto& vel = view.get<const Velocity>(entity);
            const auto& [radius, color] = view.get<const Renderable>(entity);

            // Only render entities that belong to the current scene
            if (const auto& [belongsToScene, persistent] = view.get<const SceneEntity>(entity);
                belongsToScene != currentScene) {
                continue;
            }

            const float brushRadius = radius;

            // Determine movement direction (unit vector) using velocity, fallback to rotation
            float dirAngleRad;
            bool hasMovement = false;
            if (Vector2Length(vel.velocity) > 10.0f) {  // Only show arrow when moving with significant speed
                dirAngleRad = atan2f(vel.velocity.y, vel.velocity.x);
                hasMovement = true;
            } else if (Vector2Length(vel.velocity) > 0.1f) {
                dirAngleRad = atan2f(vel.velocity.y, vel.velocity.x);
                hasMovement = false;  // Too slow, don't show arrow
            } else {
                dirAngleRad = vel.rotation * DEG2RAD;
                hasMovement = false;  // Stationary, don't show arrow
            }
            
            // Only render arrow if there's significant movement
            if (!hasMovement) {
                continue;
            }
            
            const Vector2 dirUnit { cosf(dirAngleRad), sinf(dirAngleRad) };

            // Place arrow on the brush perimeter along the movement direction
            const float arrowSize = brushRadius * 1.2f; // Make arrow larger and more visible
            const float ringRadius = brushRadius + (arrowSize * 0.2f); // Position outside brush for better visibility
            const Vector2 arrowCenter {
                position.x + dirUnit.x * ringRadius,
                position.y + dirUnit.y * ringRadius
            };

            const Rectangle destinationRect = {
                arrowCenter.x,
                arrowCenter.y,
                arrowSize,
                arrowSize
            };

            const Vector2 origin = {arrowSize / 2.0f, arrowSize / 2.0f};

            // Rotate arrow to align with up-facing texture (add +90 degrees)
            const float movementAngleDeg = dirAngleRad * RAD2DEG + 90.0f;

            renderer.drawTexture(
                arrowTexture,
                {0, 0, static_cast<float>(arrowTexture.width), static_cast<float>(arrowTexture.height)},
                destinationRect,
                origin,
                movementAngleDeg,
                RED  // Use bright red color for visibility
            );
        }
    }
};

#endif // DIDDLEDOODLEDUEL_ARROW_RENDER_H
