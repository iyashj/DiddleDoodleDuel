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
    createPlayer();
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

void DiddleDoodleDuel::createPlayer() {
    const auto player = registry.create();
    registry.emplace<Position>(player, Vector2{640.0F, 360.0F});
    registry.emplace<Velocity>(player);
    registry.emplace<Renderable>(player, 25.0F, Color{0, 229, 255, 255});
    registry.emplace<Player>(player, 250.0F);
    registry.emplace<InputAction>(player, InputAction{ .rotateLeft = false, .rotateRight = false } );
    registry.emplace<InputMapping>(player, InputMapping(KEY_A, KEY_D) );
}