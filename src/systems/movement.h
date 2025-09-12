#ifndef DIDDLEDOODLEDUEL_MOVEMENT_H
#define DIDDLEDOODLEDUEL_MOVEMENT_H
#include "../components/input_structs.h"
#include "../components/movement_structs.h"
#include <algorithm>
#include <entt/entity/registry.hpp>
#include <fmt/format.h>
#include <iostream>

struct MovementSystem {
    void update(entt::registry& registry, const float deltaTime) {
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

            velocity.x = cosf(rotation * DEG2RAD) * speed * deltaTime;
            velocity.y = sinf(rotation * DEG2RAD) * speed * deltaTime;

            position.x += velocity.x * deltaTime;
            position.y += velocity.y * deltaTime;

            position.x = std::clamp(position.x, 25.0F, 1255.0F);
            position.y = std::clamp(position.y, 25.0F, 695.0F);
        }
    }
};

#endif // DIDDLEDOODLEDUEL_MOVEMENT_H
