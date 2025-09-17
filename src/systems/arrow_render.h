#ifndef DIDDLEDOODLEDUEL_ARROW_RENDER_H
#define DIDDLEDOODLEDUEL_ARROW_RENDER_H
#include "components/position.h"
#include "components/renderable.h"
#include "components/velocity.h"
#include "rendering/irenderer.h"
#include "resources/resource_manager.h"
#include <entt/entity/registry.hpp>
#include <raylib.h>

struct ArrowRenderSystem {
    entt::registry& registry;
    engine::IRenderer& renderer;
    Texture2D arrowTexture{};

    explicit ArrowRenderSystem(entt::registry& registry, engine::IRenderer& renderer)
        : registry(registry), renderer(renderer) {
        
        if (const auto result = engine::resources::loadTexture("resources/textures/arrowFacingUp.png");
            result.has_value()) {
            this->arrowTexture = result.value();
        }
    }

    void render() const {
        for (const auto view = registry.view<const Position, const Velocity, const Renderable>();
             const auto entity : view) {
            const auto& [position] = view.get<const Position>(entity);
            const auto& vel = view.get<const Velocity>(entity);
            const auto& [radius, color] = view.get<const Renderable>(entity);

            const float brushRadius = radius;
            const Vector2 arrowPos = {
                position.x,
                position.y + brushRadius * 0.7f  // Slightly below center of brush
            };

            // Small arrow size
            const float arrowSize = brushRadius * 0.6f;
            const Rectangle destinationRect = {
                arrowPos.x,
                arrowPos.y,
                arrowSize,
                arrowSize
            };

            const Vector2 origin = {arrowSize / 2.0f, arrowSize / 2.0f};
            
            // Calculate movement direction from actual velocity vector
            float movementAngle = 0.0f;
            if (Vector2Length(vel.velocity) > 0.1f) {
                // Use actual velocity direction for accurate arrow pointing
                movementAngle = atan2f(vel.velocity.y, vel.velocity.x) * RAD2DEG;
                // Since arrowFacingUp.png points up (270° or -90°), we need to adjust
                movementAngle += 90.0f; // Rotate to align with up-facing arrow
            } else {
                // If not moving, use rotation direction
                movementAngle = vel.rotation + 90.0f;
            }

            renderer.drawTexture(
                arrowTexture,
                {0, 0, static_cast<float>(arrowTexture.width), static_cast<float>(arrowTexture.height)},
                destinationRect,
                origin,
                movementAngle,
                WHITE
            );
        }
    }
};

#endif // DIDDLEDOODLEDUEL_ARROW_RENDER_H
