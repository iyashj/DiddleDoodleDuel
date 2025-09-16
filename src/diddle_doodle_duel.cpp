#include "diddle_doodle_duel.h"
#include "components/input_action.h"
#include "components/input_mapping.h"
#include "components/renderable.h"
#include <entt/entity/registry.hpp>

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer)
: Game(renderer)
{
    SetTargetFPS(60);

    gameConfig = GameConfig {
        .brushSize = 25.0F,
        .brushMovementSpeed = 50.0F,
        .collisionForceMultiplier = 2.0F
    };

    imguiSystem = std::make_unique<ImGuiSystem>(ImGuiSystem(gameConfig));
    paintSystem = std::make_unique<PaintSystem>(PaintSystem(this->getRenderer(), gameConfig, registry));
    movementSystem = std::make_unique<MovementSystem>(MovementSystem(registry, gameConfig));
    inputSystem = std::make_unique<InputSystem>(InputSystem(registry));
    uiSystem = std::make_unique<UISystem>(this->getRenderer());
    collisionSystem = std::make_unique<CollisionSystem>(CollisionSystem(registry, gameConfig));
}

DiddleDoodleDuel::~DiddleDoodleDuel() = default;

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel - ECS Base";
    
    // Initialize ImGui
    imguiSystem->initialize();
    
    createPlayer(Vector2 {.x = 640.0F, .y = 360.0F},
        0.0F,
        KEY_A,
        KEY_D,
        GOLD);

    createPlayer(Vector2 {.x = 300.0F, .y = 360.0F},
        45.0F,
        KEY_LEFT,
        KEY_RIGHT,
        BLUE);

    createPlayer(Vector2 {.x = 640.0F, .y = 360.0F},
        90.0F,
        KEY_LEFT,
        KEY_RIGHT,
        GREEN);

    createPlayer(Vector2 {.x = 640.0F, .y = 222.0F},
        135.0F,
        KEY_A,
        KEY_D,
        RED);
}

void DiddleDoodleDuel::onUpdate(const float deltaTime) {
    this->inputSystem->update();
    this->movementSystem->update(deltaTime);
    this->paintSystem->update();
    this->collisionSystem->update(deltaTime);
}

void DiddleDoodleDuel::onRender() {
    this->paintSystem->render();
    this->uiSystem->render(title);
    
    // Render ImGui
    imguiSystem->beginFrame();
    imguiSystem->renderGameUI(title, GetFPS());
    imguiSystem->endFrame();
}

void DiddleDoodleDuel::createPlayer(
    const Vector2 startPosition,
    const float initialRotation,
    const KeyboardKey rotateLeftKey,
    const KeyboardKey rotateRightKey,
    const Color brushColor) {

    const auto player = registry.create();
    registry.emplace<Position>(player, startPosition);

    registry.emplace<Velocity>(player, Velocity{.velocity = {0,0}, .rotation = 0, .speed = gameConfig.brushMovementSpeed, .rotationSpeed = 180.0F});
    auto& velocity = registry.get<Velocity>(player);
    velocity.rotation = initialRotation;

    registry.emplace<Renderable>(player, Renderable{.radius = gameConfig.brushSize, .color = brushColor});
    registry.emplace<InputAction>(player, InputAction{ .rotateLeft = false, .rotateRight = false } );
    registry.emplace<InputMapping>(player, InputMapping{ .rotateLeftKey = rotateLeftKey, .rotateRightKey = rotateRightKey} );
    registry.emplace<CollisionState>(player, CollisionState{ .isInCollision = false, .bounceTimer = 0.0F, .bounceVelocity = Vector2 {0,0}});
}