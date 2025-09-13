#ifndef DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#define DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#include "game/game.h"
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

    std::unique_ptr<PaintSystem> paintSystem;
    std::unique_ptr<MovementSystem> movementSystem;
    std::unique_ptr<InputSystem> inputSystem;
    std::unique_ptr<UISystem> uiSystem;

    void createPlayer(Vector2 startPosition,
    KeyboardKey rotateLeftKey,
    KeyboardKey rotateRightKey,
    Color brushColor,
    float brushStrokeSize);
};

#endif // DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
