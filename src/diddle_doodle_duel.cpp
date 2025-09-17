#include "diddle_doodle_duel.h"
#include "components/input_action.h"
#include "components/input_mapping.h"

#include "components/renderable.h"
#include "systems/debug_render.h"
#include <entt/entity/registry.hpp>

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer)
: Game(renderer)
{
    SetTargetFPS(60);

    gameConfig = GameConfig {
        .brushSize = 25.0F,
        .brushMovementSpeed = 200.0F,           // Constant movement speed
        .collisionForceMultiplier = 3.0F,       // Increased bounce force multiplier
        .bounceDuration = 0.6F,                 // Longer collision cooldown time
        .controlDuringBounceFactor = 0.2F,
        .debugCollisionRadius = 25.0F,
        
        // Collision physics
        .restitution = 0.6F,
        .collisionDamping = 0.8F,
        .separationForce = 150.0F
    };

    imguiSystem = std::make_unique<ImGuiSystem>(ImGuiSystem(registry, gameConfig));
    paintSystem = std::make_unique<PaintSystem>(PaintSystem(this->getRenderer(), gameConfig, registry));
    physicsMovementSystem = std::make_unique<PhysicsMovementSystem>(PhysicsMovementSystem(registry, gameConfig));
    inputSystem = std::make_unique<InputSystem>(InputSystem(registry));
    uiSystem = std::make_unique<UISystem>(this->getRenderer());
    physicsCollisionSystem = std::make_unique<PhysicsCollisionSystem>(PhysicsCollisionSystem(registry, gameConfig));
    debugRenderSystem = std::make_unique<DebugRenderSystem>(registry, gameConfig);
    arrowRenderSystem = std::make_unique<ArrowRenderSystem>(registry, this->getRenderer());
}

DiddleDoodleDuel::~DiddleDoodleDuel() = default;

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel - ECS Base";
    
    // Initialize ImGui
    imguiSystem->initialize();
    
    // Create four brushes at corners with preset directions pointing toward center
    createPlayer(Vector2 {.x = 200.0F, .y = 200.0F},    // Top-left
        315.0F,  // Point toward center (southeast)
        KEY_A,
        KEY_D,
        GOLD);

    createPlayer(Vector2 {.x = 1080.0F, .y = 200.0F},   // Top-right
        225.0F,  // Point toward center (southwest)
        KEY_LEFT,
        KEY_RIGHT,
        BLUE);

    createPlayer(Vector2 {.x = 1080.0F, .y = 520.0F},   // Bottom-right
        135.0F,  // Point toward center (northwest)
        KEY_J,
        KEY_L,
        GREEN);

    createPlayer(Vector2 {.x = 200.0F, .y = 520.0F},    // Bottom-left
        45.0F,   // Point toward center (northeast)
        KEY_F,
        KEY_H,
        RED);
}

void DiddleDoodleDuel::onUpdate(const float deltaTime) {
    this->inputSystem->update();
    // Use physics-based movement instead of the old movement system
    this->physicsMovementSystem->update(deltaTime);
    this->paintSystem->update();
    // Use physics-based collision instead of the old collision system
    this->physicsCollisionSystem->update(deltaTime);
}

void DiddleDoodleDuel::onRender() {
    this->paintSystem->render();
    this->arrowRenderSystem->render();
    this->uiSystem->render(title);

    if (imguiSystem->isDebugWindowVisible()) {
        debugRenderSystem->render();
    }
    
    // Render ImGui
    imguiSystem->beginFrame();
    imguiSystem->renderGameUI(title, GetFPS());
    imguiSystem->renderEcsDebug();
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

    registry.emplace<Velocity>(player, Velocity{.velocity = {0,0}, .rotation = initialRotation, .speed = gameConfig.brushMovementSpeed, .rotationSpeed = 120.0F});

    registry.emplace<Renderable>(player, Renderable{.radius = gameConfig.brushSize, .color = brushColor});
    registry.emplace<InputAction>(player, InputAction{ .rotateLeft = false, .rotateRight = false } );
    registry.emplace<InputMapping>(player, InputMapping{ .rotateLeftKey = rotateLeftKey, .rotateRightKey = rotateRightKey} );
    registry.emplace<CollisionState>(player, CollisionState{ .isInCollision = false, .bounceTimer = 0.0F, .bounceVelocity = Vector2 {0,0}});
}