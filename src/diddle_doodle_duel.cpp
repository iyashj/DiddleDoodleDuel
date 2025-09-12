#include "diddle_doodle_duel.h"
#include "components/drawing_structs.h"
#include "components/input_structs.h"
#include "components/movement_structs.h"
#include "components/player.h"
#include <entt/entity/registry.hpp>

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer) : Game(renderer), paintSystem(PaintSystem(canvasPixel, renderer)) {
    SetTargetFPS(60);
}

DiddleDoodleDuel::~DiddleDoodleDuel() = default;

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel - ECS Base";

    const auto player = registry.create();
    registry.emplace<Position>(player, Vector2{640.0F, 360.0F});
    registry.emplace<Velocity>(player);
    registry.emplace<Renderable>(player, 25.0F, Color{0, 229, 255, 255});
    registry.emplace<Player>(player, 250.0F);
    registry.emplace<InputAction>(player, InputAction{ .rotateLeft = false, .rotateRight = false } );
    registry.emplace<InputMapping>(player, InputMapping(KEY_A, KEY_D) );

    initializeCanvasBuffer();
}

void DiddleDoodleDuel::initializeCanvasBuffer() {
    const auto size = static_cast<size_t>(getRenderer().getWindowWidth() * getRenderer().getWindowHeight());
    canvasPixel.assign(size, BLANK);
}

void DiddleDoodleDuel::onUpdate(const float deltaTime) {
    inputSystem.update(registry);
    movementSystem.update(registry, deltaTime);
    paintSystem.update(registry);
}

void DiddleDoodleDuel::createPlayers() {}

void DiddleDoodleDuel::onRender() {
    uiSystem.render(this->getRenderer(), title);
    paintSystem.render(registry, this->getRenderer());
}