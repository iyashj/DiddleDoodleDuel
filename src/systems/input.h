#ifndef DIDDLEDOODLEDUEL_INPUT_H
#define DIDDLEDOODLEDUEL_INPUT_H
#include "../components/input_structs.h"
#include "../components/player.h"
#include <entt/entity/registry.hpp>

struct InputSystem {
    void update(entt::registry& registry) {
        const auto players = registry.view<InputAction, InputMapping>();
        players.each([&](InputAction& inputAction, const InputMapping& inputMapping) {
            inputAction.rotateLeft = IsKeyDown(inputMapping.rotateLeftKey);
            inputAction.rotateRight = IsKeyDown(inputMapping.rotateRightKey);
        });
    }
};

#endif // DIDDLEDOODLEDUEL_INPUT_H
