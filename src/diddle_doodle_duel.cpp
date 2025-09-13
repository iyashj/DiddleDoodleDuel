#include "diddle_doodle_duel.h"
#include "components/drawing_structs.h"
#include "components/input_structs.h"
#include "components/movement_structs.h"
#include "components/player.h"
#include <entt/entity/registry.hpp>

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer)
: Game(renderer)
{
    SetTargetFPS(60);
    paintSystem = std::make_unique<PaintSystem>(renderer);
}

DiddleDoodleDuel::~DiddleDoodleDuel() = default;

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel - ECS Base";
    createPlayer(Vector2 {.x = 640.0F, .y = 360.0F},
        KEY_A,
        KEY_D,
        GOLD,
        25.0F);

    createPlayer(Vector2 {.x = 300.0F, .y = 360.0F},
        KEY_LEFT,
        KEY_RIGHT,
        BLUE,
        25.0F);

    createPlayer(Vector2 {.x = 640.0F, .y = 360.0F},
        KEY_LEFT,
        KEY_RIGHT,
        GREEN,
        25.0f);

    createPlayer(Vector2 {.x = 640.0F, .y = 222.0F},
        KEY_A,
        KEY_D,
        RED,
        25.0f);
}

void DiddleDoodleDuel::onUpdate(const float deltaTime) {
    this->inputSystem->update(registry);
    this->movementSystem->update(registry, deltaTime);
    this->paintSystem->update(registry);
}

void DiddleDoodleDuel::onRender() {
    this->paintSystem->render();
    this->uiSystem->render(this->getRenderer(), title);
}

void DiddleDoodleDuel::createPlayer(
    Vector2 startPosition,
    const KeyboardKey rotateLeftKey,
    const KeyboardKey rotateRightKey,
    const Color brushColor,
    const float brushStrokeSize) {

    const auto player = registry.create();
    registry.emplace<Position>(player, startPosition);
    registry.emplace<Velocity>(player);
    registry.emplace<Renderable>(player, brushStrokeSize, brushColor);
    registry.emplace<InputAction>(player, InputAction{ .rotateLeft = false, .rotateRight = false } );
    registry.emplace<InputMapping>(player, InputMapping(rotateLeftKey, rotateRightKey) );
}