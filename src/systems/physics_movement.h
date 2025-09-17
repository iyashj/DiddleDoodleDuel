#ifndef DIDDLEDOODLEDUEL_PHYSICS_MOVEMENT_H
#define DIDDLEDOODLEDUEL_PHYSICS_MOVEMENT_H
#include "components/collision_state.h"

#include "game_config.h"
#include <algorithm>
#include <cmath>
#include <entt/entity/registry.hpp>

struct PhysicsMovementSystem {
    explicit PhysicsMovementSystem(entt::registry& registry, GameConfig& config)
    : registry(registry), config(config)
    {
    }

    void update(const float deltaTime) {
        handleInput(deltaTime);
        integratePhysics(deltaTime);
        constrainToBounds();
    }

private:
    entt::registry& registry;
    const GameConfig& config;

    void handleInput(const float deltaTime) const {
        for (const auto view = registry.view<Velocity, InputAction>();
            const auto entity : view) {
            
            const auto& [rotateLeft, rotateRight] = view.get<InputAction>(entity);
            auto& velocity = view.get<Velocity>(entity);

            // Apply turning forces (angular velocity control)
            if (rotateLeft) {
                velocity.rotation -= velocity.rotationSpeed * deltaTime;
            }
            if (rotateRight) {
                velocity.rotation += velocity.rotationSpeed * deltaTime;
            }

            // Continuous forward movement - always thrust in current direction
            const float thrustAngle = velocity.rotation * DEG2RAD;
            velocity.velocity.x = cosf(thrustAngle) * config.brushMovementSpeed;
            velocity.velocity.y = sinf(thrustAngle) * config.brushMovementSpeed;
        }
    }

    void integratePhysics(const float deltaTime) {
        for (const auto view = registry.view<Position, Velocity>();
            const auto entity : view) {
            
            auto& position = view.get<Position>(entity);
            auto& velocity = view.get<Velocity>(entity);

            // Check collision state - during collision, apply strong bounce forces
            const CollisionState* col = registry.try_get<CollisionState>(entity);
            if (col && col->isInCollision && col->bounceTimer > 0.0f) {
                // Override normal movement with bounce velocity for more impact
                position.position.x += col->bounceVelocity.x * deltaTime;
                position.position.y += col->bounceVelocity.y * deltaTime;
            } else {
                // Normal movement - continuous forward motion
                position.position.x += velocity.velocity.x * deltaTime;
                position.position.y += velocity.velocity.y * deltaTime;
            }
        }
    }

    void constrainToBounds() {
        for (const auto view = registry.view<Position, Velocity>();
            const auto entity : view) {
            
            auto& position = view.get<Position>(entity);

            const float margin = config.brushSize;
            const float minX = margin;
            const float maxX = 1280.0f - margin;  // Screen width bounds
            const float minY = margin;
            const float maxY = 720.0f - margin;   // Screen height bounds

            // Clamp to screen bounds (no bouncing, just constraint)
            position.position.x = std::clamp(position.position.x, minX, maxX);
            position.position.y = std::clamp(position.position.y, minY, maxY);
        }
    }
};

#endif // DIDDLEDOODLEDUEL_PHYSICS_MOVEMENT_H