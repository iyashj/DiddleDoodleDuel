#ifndef DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#define DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#include "core/event_bus.h"
#include "core/event_definitions.h"
#include "game/game.h"
#include "game_config.h"
#include "network/multiplayer_manager.h"
#include "systems/arrow_render.h"
#include "systems/collision.h"
#include "systems/debug_render.h"
#include "systems/entity_lifecycle_system.h"
#include "systems/imgui_system.h"
#include "systems/input.h"
#include "systems/paint.h"
#include "systems/physics_collision.h"
#include "systems/physics_movement.h"
#include "systems/scene_transition_system.h"
#include "systems/system_activation_system.h"
#include "systems/ui.h"
#include "systems/username_render.h"
#include <entt/entity/registry.hpp>

class DiddleDoodleDuel : public engine::Game {
    void onMenuEvent(const MenuEvent& evt);
    void onMultiplayerEvent(const MultiplayerEvent& evt);

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

    std::unique_ptr<EventBus> eventBus;
    std::unique_ptr<PaintSystem> paintSystem;
    std::unique_ptr<PhysicsMovementSystem> physicsMovementSystem;
    std::unique_ptr<InputSystem> inputSystem;
    std::unique_ptr<UISystem> uiSystem;
    std::unique_ptr<PhysicsCollisionSystem> physicsCollisionSystem;
    std::unique_ptr<DebugRenderSystem> debugRenderSystem;
    std::unique_ptr<ArrowRenderSystem> arrowRenderSystem;
    std::unique_ptr<UsernameRenderSystem> usernameRenderSystem;
    std::unique_ptr<ImGuiSystem> imguiSystem;
    std::unique_ptr<network::MultiplayerManager> multiplayerManager;

    // UI state for multiplayer
    char serverAddress[256] = "127.0.0.1";
    int serverPort = 7777;
    char playerUsername[64] = "Player";
    Color selectedColor = BLUE;
    bool isConnecting = false;

    void createPlayer(
        Vector2 startPosition,
        float initialRotation,
        KeyboardKey rotateLeftKey,
        KeyboardKey rotateRightKey,
        Color brushColor);

    void startLocalGame();
    void renderMainMenuUI() const;
    void renderOnlineUI();
    void renderLobbyUI();

    void executeUpdateOnActiveSystems(float deltaTime) const;
    void executeRenderOnWorldSystems() const;

    void handleInputEvents() const;
    void renderUISystems(SceneType currentScene);
    void renderDebugInfo(SceneType currentScene) const;
    
    void handleMultiplayerInput() const;
};

#endif // DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
