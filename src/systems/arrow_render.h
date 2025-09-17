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

            // Determine movement direction (unit vector) using velocity, fallback to rotation
            float dirAngleRad;
            if (Vector2Length(vel.velocity) > 0.1f) {
                dirAngleRad = atan2f(vel.velocity.y, vel.velocity.x);
            } else {
                dirAngleRad = vel.rotation * DEG2RAD;
            }
            const Vector2 dirUnit { cosf(dirAngleRad), sinf(dirAngleRad) };

            // Place arrow on the brush perimeter along the movement direction
            const float arrowSize = brushRadius * 0.6f; // small arrow
            const float ringRadius = brushRadius - (arrowSize * 0.15f); // slight inset to avoid clipping
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
                WHITE
            );
        }
    }
};

#endif // DIDDLEDOODLEDUEL_ARROW_RENDER_H
