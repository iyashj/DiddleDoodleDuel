#ifndef DIDDLEDOODLEDUEL_MOVEMENT_H
#define DIDDLEDOODLEDUEL_MOVEMENT_H

#include <algorithm>
#include <entt/entity/registry.hpp>
#include <fmt/format.h>
#include <iostream>
#include "game_config.h"
#include "components/collision_state.h"

struct MovementSystem {
    explicit MovementSystem(entt::registry& registry, GameConfig& config)
    : registry(registry), config(config)
    {
    }

    void update(const float deltaTime) {
        for (const auto movementView = registry.view<Position, Velocity, InputAction>();
            const auto entity : movementView) {
            const auto& [rotateLeft, rotateRight] = movementView.get<InputAction>(entity);
            auto& [position] = movementView.get<Position>(entity);
            auto& [velocity, rotationSpeed, speed, rotation] = movementView.get<Velocity>(entity);

            if (rotateLeft) {
                rotation -= rotationSpeed * deltaTime;
            }

            if (rotateRight) {
                rotation += rotationSpeed * deltaTime;
            }

            velocity.x = cosf(rotation * DEG2RAD) * config.brushMovementSpeed * deltaTime;
            velocity.y = sinf(rotation * DEG2RAD) * config.brushMovementSpeed * deltaTime;

            // Blend user control while bouncing: reduced control factor during bounce
            float controlFactor = 1.0f;
            const CollisionState* col = registry.try_get<CollisionState>(entity);
            if (col && col->isInCollision && col->bounceTimer > 0.0f) {
                controlFactor = config.controlDuringBounceFactor; // keep some control but let impulse dominate
            }

            position.x += velocity.x * controlFactor;
            position.y += velocity.y * controlFactor;

            // Apply bounce impulse if currently in collision bounce state
            if (col && col->isInCollision && col->bounceTimer > 0.0f) {
                position.x += col->bounceVelocity.x * deltaTime;
                position.y += col->bounceVelocity.y * deltaTime;
            }

            position.x = std::clamp(position.x, 25.0F, 1255.0F);
            position.y = std::clamp(position.y, 25.0F, 695.0F);
        }
    }

private:
    entt::registry& registry;
    const GameConfig& config;
};

#endif // DIDDLEDOODLEDUEL_MOVEMENT_H
