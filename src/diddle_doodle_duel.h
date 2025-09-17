#ifndef DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#define DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#include "game/game.h"
#include "game_config.h"
#include "systems/collision.h"
#include "systems/debug_render.h"
#include "systems/imgui_system.h"
#include "systems/input.h"
#include "systems/movement.h"
#include "systems/paint.h"
#include "systems/ui.h"
#include <entt/entity/registry.hpp>

class DiddleDoodleDuel : public engine::Game {
public:
    explicit DiddleDoodleDuel(engine::IRenderer& renderer);
    ~DiddleDoodleDuel() override;
    void onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    entt::registry registry;
    std::string title;

    GameConfig gameConfig;

    std::unique_ptr<PaintSystem> paintSystem;
    std::unique_ptr<MovementSystem> movementSystem;
    std::unique_ptr<InputSystem> inputSystem;
    std::unique_ptr<UISystem> uiSystem;
    std::unique_ptr<CollisionSystem> collisionSystem;
    std::unique_ptr<DebugRenderSystem> debugRenderSystem;
    std::unique_ptr<ImGuiSystem> imguiSystem;

    void createPlayer(
        Vector2 startPosition,
        float initialRotation,
        KeyboardKey rotateLeftKey,
        KeyboardKey rotateRightKey,
        Color brushColor);
};

#endif // DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
