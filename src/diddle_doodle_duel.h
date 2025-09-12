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
    void initializeCanvasBuffer();
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    entt::registry registry;
    std::string title;
    std::vector<Color> canvasPixel;

    PaintSystem paintSystem;
    MovementSystem movementSystem;
    InputSystem inputSystem;
    UISystem uiSystem;

    void checkBoundsAgainstScreen();
    void createPlayers();
};

#endif // DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
