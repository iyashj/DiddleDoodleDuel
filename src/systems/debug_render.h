#ifndef DIDDLEDOODLEDUEL_DEBUG_RENDER_H
#define DIDDLEDOODLEDUEL_DEBUG_RENDER_H

#include "game_config.h"
#include "components/position.h"
#include <entt/entity/registry.hpp>
#include <raylib.h>

struct DebugRenderSystem {
    explicit DebugRenderSystem(entt::registry& registry, const GameConfig& config)
        : registry(registry), config(config) {}

    void render() const {
        const auto view = registry.view<const Position>();
        for (const auto entity : view) {
            const auto& pos = view.get<const Position>(entity);
            DrawCircleLines(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), config.debugCollisionRadius, RED);
        }
    }

private:
    entt::registry& registry;
    const GameConfig& config;
};

#endif // DIDDLEDOODLEDUEL_DEBUG_RENDER_H
