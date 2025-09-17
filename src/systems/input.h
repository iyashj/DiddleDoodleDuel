#ifndef DIDDLEDOODLEDUEL_INPUT_H
#define DIDDLEDOODLEDUEL_INPUT_H
#include "components/input_action.h"
#include "components/input_mapping.h"
#include "components/collision_state.h"
#include <entt/entity/registry.hpp>

struct InputSystem {
    explicit InputSystem(entt::registry& registry) : registry(registry) {
    }

    void update() {
        const auto players = registry.view<InputAction, InputMapping>();
        players.each([&](InputAction& inputAction, const InputMapping& inputMapping) {
            inputAction.rotateLeft = IsKeyDown(inputMapping.rotateLeftKey);
            inputAction.rotateRight = IsKeyDown(inputMapping.rotateRightKey);
        });
    }

private:
    entt::registry& registry;
};

#endif // DIDDLEDOODLEDUEL_INPUT_H
