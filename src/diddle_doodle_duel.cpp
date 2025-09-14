#include "diddle_doodle_duel.h"
#include "components/input_action.h"
#include "components/input_mapping.h"
#include "components/renderable.h"
#include <entt/entity/registry.hpp>

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer)
: Game(renderer)
{
    SetTargetFPS(60);

    paintSystem = std::make_unique<PaintSystem>(renderer);
    movementSystem = std::make_unique<MovementSystem>();
    inputSystem = std::make_unique<InputSystem>();
    uiSystem = std::make_unique<UISystem>();
    collisionSystem = std::make_unique<CollisionSystem>();
}

DiddleDoodleDuel::~DiddleDoodleDuel() = default;

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel - ECS Base";
    createPlayer(Vector2 {.x = 640.0F, .y = 360.0F},
        0.0F,
        KEY_A,
        KEY_D,
        GOLD,
        25.0F);

    createPlayer(Vector2 {.x = 300.0F, .y = 360.0F},
        45.0F,
        KEY_LEFT,
        KEY_RIGHT,
        BLUE,
        25.0F);

    createPlayer(Vector2 {.x = 640.0F, .y = 360.0F},
        90.0F,
        KEY_LEFT,
        KEY_RIGHT,
        GREEN,
        25.0f);

    createPlayer(Vector2 {.x = 640.0F, .y = 222.0F},
        135.0F,
        KEY_A,
        KEY_D,
        RED,
        25.0f);
}

void DiddleDoodleDuel::onUpdate(const float deltaTime) {
    this->inputSystem->update(registry);
    this->movementSystem->update(registry, deltaTime);
    this->paintSystem->update(registry);
    this->collisionSystem->update(registry, deltaTime);
}

void DiddleDoodleDuel::onRender() {
    this->paintSystem->render(this->registry, this->getRenderer());
    this->uiSystem->render(this->getRenderer(), title);
}

void DiddleDoodleDuel::createPlayer(
    const Vector2 startPosition,
    const float initialRotation,
    const KeyboardKey rotateLeftKey,
    const KeyboardKey rotateRightKey,
    const Color brushColor,
    const float brushStrokeSize) {

    const auto player = registry.create();
    registry.emplace<Position>(player, startPosition);

    registry.emplace<Velocity>(player, Velocity{.velocity = {0,0}, .rotation = 0, .speed = 5000.0F, .rotationSpeed = 180.0F});
    auto& velocity = registry.get<Velocity>(player);
    velocity.rotation = initialRotation;

    registry.emplace<Renderable>(player, Renderable{.radius = brushStrokeSize, .color = brushColor});
    registry.emplace<InputAction>(player, InputAction{ .rotateLeft = false, .rotateRight = false } );
    registry.emplace<InputMapping>(player, InputMapping{ .rotateLeftKey = rotateLeftKey, .rotateRightKey = rotateRightKey} );
    registry.emplace<CollisionState>(player, CollisionState{ .isInCollision = false, .bounceTimer = 0.0F, .bounceVelocity = Vector2 {0,0}});
}