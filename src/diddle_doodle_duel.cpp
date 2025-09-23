#include "diddle_doodle_duel.h"
#include "components/collision_state.h"
#include "components/input_action.h"
#include "components/input_mapping.h"
#include "components/position.h"
#include "components/renderable.h"
#include "components/velocity.h"
#include "logging/logger.h"
#include "systems/debug_render.h"
#include "performance/profiler.h"
#include <entt/entity/registry.hpp>
#include <iostream>

void DiddleDoodleDuel::onMenuEvent(const MenuEvent& evt) {
    switch (evt.type) {
        case MenuEvent::Type::StartLocalGame:
            startLocalGame();
            break;
        case MenuEvent::Type::StartOnlineGame:
            SceneTransitionSystem::requestTransition(registry, SceneType::NetworkingDemo);
            break;
        case MenuEvent::Type::ExitGame:
            LOG_INFO_MSG("User requested game exit (event)");
            CloseWindow();
            break;
        case MenuEvent::Type::BackToMenu:
            if (multiplayerManager) {
                multiplayerManager->disconnect();
            }
            SceneTransitionSystem::requestTransition(registry, SceneType::MainMenu);
            break;
    }
}

void DiddleDoodleDuel::onMultiplayerEvent(const MultiplayerEvent& evt) {
    switch (evt.type) {
        case MultiplayerEvent::Type::StartServer:
            if (multiplayerManager) {
                multiplayerManager->setPlayerInfo(std::string(playerUsername), selectedColor);
                if (multiplayerManager->startServer(static_cast<uint16_t>(serverPort))) {
                    SceneTransitionSystem::requestTransition(registry, SceneType::Lobby);
                }
            }
            break;
        case MultiplayerEvent::Type::ConnectToServer:
            if (multiplayerManager) {
                isConnecting = true;
                multiplayerManager->setPlayerInfo(std::string(playerUsername), selectedColor);
                if (multiplayerManager->connectToServer(std::string(serverAddress), static_cast<uint16_t>(serverPort))) {
                    // Connection attempt started, wait for status update
                } else {
                    isConnecting = false;
                }
            }
            break;
        case MultiplayerEvent::Type::Disconnect:
            if (multiplayerManager) {
                multiplayerManager->disconnect();
            }
            SceneTransitionSystem::requestTransition(registry, SceneType::MainMenu);
            break;
        case MultiplayerEvent::Type::LobbyUpdate:
            // Handle lobby updates if needed
            break;
        case MultiplayerEvent::Type::GameStart:
            SceneTransitionSystem::requestTransition(registry, SceneType::NetworkedGame);
            break;
    }
}

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer) : Game(renderer) {
    SetTargetFPS(60);

    engine::resources::setResourceRoot(engine::resources::getExecutableDir() / "resources");

    gameConfig = GameConfig{.brushSize = 25.0F,
                            .brushMovementSpeed = 200.0F,
                            .collisionForceMultiplier = 3.0F,
                            .bounceDuration = 0.6F,
                            .controlDuringBounceFactor = 0.2F,
                            .debugCollisionRadius = 25.0F,
                            .restitution = 0.6F,
                            .collisionDamping = 0.8F,
                            .separationForce = 150.0F};

    eventBus = std::make_unique<EventBus>();
    SceneTransitionSystem::initializeSceneState(registry);

    imguiSystem = std::make_unique<ImGuiSystem>(ImGuiSystem(registry, gameConfig));
    
    // Initialize multiplayer manager first
    multiplayerManager = std::make_unique<network::MultiplayerManager>(registry, gameConfig);
    
    // Now initialize paint system with multiplayer manager
    paintSystem = std::make_unique<PaintSystem>(PaintSystem(this->getRenderer(), gameConfig, registry, multiplayerManager.get()));
    
    physicsMovementSystem =
        std::make_unique<PhysicsMovementSystem>(PhysicsMovementSystem(registry, gameConfig));
    inputSystem = std::make_unique<InputSystem>(InputSystem(registry));
    uiSystem = std::make_unique<UISystem>(this->getRenderer());
    physicsCollisionSystem =
        std::make_unique<PhysicsCollisionSystem>(PhysicsCollisionSystem(registry, gameConfig));
    debugRenderSystem = std::make_unique<DebugRenderSystem>(registry, gameConfig);
    arrowRenderSystem = std::make_unique<ArrowRenderSystem>(registry, this->getRenderer());
    usernameRenderSystem = std::make_unique<UsernameRenderSystem>(registry, this->getRenderer());
    
    // Set up multiplayer event handlers
    multiplayerManager->setStateChangeHandler([this](network::MultiplayerState state) {
        switch (state) {
            case network::MultiplayerState::InLobby:
                if (isConnecting) {
                    isConnecting = false;
                    SceneTransitionSystem::requestTransition(registry, SceneType::Lobby);
                }
                break;
            case network::MultiplayerState::InGame:
                if (eventBus) {
                    eventBus->dispatcher.trigger<MultiplayerEvent>(MultiplayerEvent{
                        MultiplayerEvent::Type::GameStart, ""
                    });
                }
                break;
            case network::MultiplayerState::Disconnected:
                isConnecting = false;
                break;
            default:
                break;
        }
    });
    
    multiplayerManager->setLobbyUpdateHandler([this](const network::LobbyState& lobbyState) {
        if (eventBus) {
            eventBus->dispatcher.trigger<MultiplayerEvent>(MultiplayerEvent{
                MultiplayerEvent::Type::LobbyUpdate, ""
            });
        }
    });
}

DiddleDoodleDuel::~DiddleDoodleDuel() {
    LOG_DEBUG_MSG("Cleaning up game resources...");
    
    // Print final performance report (only if enabled)
    if (gameConfig.enableProfiler) {
        SimpleProfiler::getInstance().printResults();
    }
    
    EntityLifecycleSystem::cleanupAllEntities(registry);
    
    if (imguiSystem) {
        imguiSystem.reset();
    }
    
    LOG_DEBUG_MSG("Game cleanup complete");
}

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel";
    LOG_DEBUG_MSG("Initializing game...");

    imguiSystem->initialize();
    LOG_DEBUG_MSG("Initializing ImGuiSystem...");

    if (eventBus) {
        eventBus->dispatcher.sink<MenuEvent>().connect<&DiddleDoodleDuel::onMenuEvent>(this);
        eventBus->dispatcher.sink<MultiplayerEvent>().connect<&DiddleDoodleDuel::onMultiplayerEvent>(this);
    }

    LOG_DEBUG_MSG("Requesting transition to MainMenu scene...");
    SceneTransitionSystem::requestTransition(registry, SceneType::MainMenu);
    LOG_DEBUG_MSG("Scene transition requested");
}

void DiddleDoodleDuel::onUpdate(const float deltaTime) {
    if (const auto& state = registry.ctx().get<SceneState>(); state.isTransitioning) {
        SceneTransitionSystem::processTransitions(registry, deltaTime);
    }

    handleInputEvents();
    executeUpdateOnActiveSystems(deltaTime);
    
    // Update multiplayer manager
    if (multiplayerManager) {
        multiplayerManager->update(deltaTime);
        
        // Handle multiplayer input for networked game
        const auto currentScene = SceneTransitionSystem::getCurrentScene(registry);
        if (currentScene == SceneType::NetworkedGame) {
            handleMultiplayerInput();
        }
    }
    
    // Manual FPS calculation to bypass display limitations
    // FPS counter (only if enabled)
    if (gameConfig.enableFpsCounter) {
        static float frameCount = 0;
        static float timeAccumulator = 0;
        frameCount++;
        timeAccumulator += deltaTime;
        
        if (timeAccumulator >= 1.0f) {
            LOG_DEBUG_MSG("TRUE FPS: %d (Frame time: %.2fms)", frameCount, (timeAccumulator/frameCount)*1000);
            frameCount = 0;
            timeAccumulator = 0;
        }
    }
    
    // Print performance stats every 5 seconds (only if enabled)
    if (gameConfig.enableProfiler) {
        static float timeSinceLastProfile = 0.0f;
        timeSinceLastProfile += deltaTime;
        if (timeSinceLastProfile >= 5.0f) {
            SimpleProfiler::getInstance().printResults();
            SimpleProfiler::getInstance().reset();
            timeSinceLastProfile = 0.0f;
        }
    }
}

void DiddleDoodleDuel::onRender() {
    SimpleProfiler::getInstance().startTimer("FullFrame");
    SimpleProfiler::getInstance().startTimer("Rendering");
    
    ClearBackground({30, 30, 40, 255});
    
    const SceneType currentScene = SceneTransitionSystem::getCurrentScene(registry);
    
    executeRenderOnWorldSystems();
    renderUISystems(currentScene);
    renderDebugInfo(currentScene);
    
    SimpleProfiler::getInstance().endTimer("Rendering");
    SimpleProfiler::getInstance().endTimer("FullFrame");
}

void DiddleDoodleDuel::createPlayer(const Vector2 startPosition, const float initialRotation,
                                    const KeyboardKey rotateLeftKey,
                                    const KeyboardKey rotateRightKey, const Color brushColor) {
    const auto player = registry.create();
    registry.emplace<Position>(player, Position{.position = startPosition});

    registry.emplace<Velocity>(player, Velocity{.velocity = {0, 0},
                                                .rotation = initialRotation,
                                                .speed = gameConfig.brushMovementSpeed,
                                                .rotationSpeed = 120.0F});

    registry.emplace<Renderable>(player,
                                 Renderable{.radius = gameConfig.brushSize, .color = brushColor});
    registry.emplace<InputAction>(player, InputAction{.rotateLeft = false, .rotateRight = false});
    registry.emplace<InputMapping>(
        player, InputMapping{.rotateLeftKey = rotateLeftKey, .rotateRightKey = rotateRightKey});
    registry.emplace<CollisionState>(player, CollisionState{.isInCollision = false,
                                                            .bounceTimer = 0.0F,
                                                            .bounceVelocity = Vector2{0, 0}});

    EntityLifecycleSystem::tagEntityWithScene(registry, player, SceneType::Game);
}

void DiddleDoodleDuel::startLocalGame() {
    EntityLifecycleSystem::cleanupSceneEntities(registry,
                                                SceneTransitionSystem::getCurrentScene(registry));

    SceneTransitionSystem::requestTransition(registry, SceneType::Game);

    createPlayer({100, 100}, 0, KEY_A, KEY_D, RED);
    createPlayer({1180, 100}, 90, KEY_LEFT, KEY_RIGHT, BLUE);
    createPlayer({1180, 620}, 180, KEY_J, KEY_L, GREEN);
    createPlayer({100, 620}, 270, KEY_F, KEY_H, YELLOW);
}

void DiddleDoodleDuel::renderMainMenuUI() const {
    static bool debugPrinted = false;
    if (!debugPrinted) {
        LOG_DEBUG_MSG("DEBUG: Rendering Main Menu UI!");
        debugPrinted = true;
    }

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    draw_list->AddRectFilled(
        viewport->Pos,
        ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y),
        IM_COL32(30, 30, 40, 255));

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f,
                                   viewport->Pos.y + viewport->Size.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 340), ImGuiCond_FirstUseEver);

    ImGui::Begin("Diddle Doodle Duel", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGui::SetCursorPosX((windowSize.x - 120) * 0.5f);
    ImGui::Dummy(ImVec2(120, 60));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 60);
    ImGui::SetCursorPosX((windowSize.x - 120) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.3f, 1.0f));
    ImGui::BeginChild("LogoBox", ImVec2(120, 60), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPosY(18);
    ImGui::SetCursorPosX(10);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "LOGO");
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    ImGui::SetWindowFontScale(2.0f);
    ImVec2 titleSize = ImGui::CalcTextSize("Diddle Doodle Duel");
    ImGui::SetCursorPosX((windowSize.x - titleSize.x) * 0.5f);
    ImGui::Text("Diddle Doodle Duel");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImVec2 buttonSize(200, 40);
    float buttonX = (windowSize.x - buttonSize.x) * 0.5f;

    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Local Game", buttonSize)) {
        if (eventBus)
            eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::StartLocalGame});
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Start a local game");
    }

    ImGui::Spacing();
    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Online Game", buttonSize)) {
        if (eventBus)
            eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::StartOnlineGame});
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Online multiplayer (WIP)");
    }

    ImGui::Spacing();
    ImGui::SetCursorPosX(buttonX);
    if (ImGui::Button("Exit", buttonSize)) {
        if (eventBus)
            eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::ExitGame});
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Exit the game");
    }

    ImGui::End();
}

void DiddleDoodleDuel::renderOnlineUI() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f,
                                   viewport->Pos.y + viewport->Size.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

    ImGui::Begin("Online Game", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImGui::Text("Online Multiplayer");
    ImGui::Separator();
    ImGui::Spacing();

    // Player settings
    ImGui::Text("Player Settings");
    ImGui::InputText("Username", playerUsername, sizeof(playerUsername));
    
    // Color picker
    ImGui::Text("Player Color:");
    static const Color predefinedColors[] = {RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE};
    static const char* colorNames[] = {"Red", "Blue", "Green", "Yellow", "Purple", "Orange"};
    static int selectedColorIndex = 1; // Default to blue
    
    for (int i = 0; i < 6; i++) {
        if (i > 0) ImGui::SameLine();
        
        ImVec4 color = ImVec4(
            predefinedColors[i].r / 255.0f,
            predefinedColors[i].g / 255.0f,
            predefinedColors[i].b / 255.0f,
            1.0f
        );
        
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 1.2f, color.y * 1.2f, color.z * 1.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, 1.0f));
        
        if (ImGui::Button(colorNames[i], ImVec2(50, 30))) {
            selectedColorIndex = i;
            selectedColor = predefinedColors[i];
        }
        
        if (selectedColorIndex == i) {
            ImGui::GetWindowDrawList()->AddRect(
                ImGui::GetItemRectMin(),
                ImGui::GetItemRectMax(),
                IM_COL32(255, 255, 255, 255),
                0.0f, 0, 2.0f
            );
        }
        
        ImGui::PopStyleColor(3);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Connection settings
    ImGui::Text("Connection Settings");
    ImGui::InputText("Server Address", serverAddress, sizeof(serverAddress));
    ImGui::InputInt("Port", &serverPort);
    
    if (serverPort < 1 || serverPort > 65535) {
        serverPort = 7777;
    }
    
    ImGui::Spacing();
    
    // Connection status
    if (multiplayerManager) {
        auto status = multiplayerManager->getConnectionStatus();
        switch (status) {
            case network::ConnectionStatus::Disconnected:
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: Disconnected");
                break;
            case network::ConnectionStatus::Connecting:
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status: Connecting...");
                break;
            case network::ConnectionStatus::Connected:
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Connected");
                break;
            case network::ConnectionStatus::Failed:
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: Connection Failed");
                break;
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Action buttons
    ImVec2 buttonSize(180, 40);
    
    if (ImGui::Button("Start Server", buttonSize)) {
        if (eventBus) {
            eventBus->dispatcher.trigger<MultiplayerEvent>(MultiplayerEvent{
                MultiplayerEvent::Type::StartServer, ""
            });
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Start a server for others to join");
    }
    
    ImGui::SameLine();
    
    bool canConnect = !isConnecting && 
                     (multiplayerManager && multiplayerManager->getConnectionStatus() == network::ConnectionStatus::Disconnected);
    
    if (!canConnect) ImGui::BeginDisabled();
    if (ImGui::Button("Connect to Server", buttonSize)) {
        if (eventBus) {
            eventBus->dispatcher.trigger<MultiplayerEvent>(MultiplayerEvent{
                MultiplayerEvent::Type::ConnectToServer, ""
            });
        }
    }
    if (!canConnect) ImGui::EndDisabled();
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Connect to an existing server");
    }
    
    ImGui::Spacing();
    
    if (isConnecting) {
        if (ImGui::Button("Cancel", ImVec2(100, 30))) {
            if (multiplayerManager) {
                multiplayerManager->disconnect();
            }
            isConnecting = false;
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Back to Main Menu", ImVec2(200, 30))) {
        if (multiplayerManager) {
            multiplayerManager->disconnect();
        }
        if (eventBus) {
            eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::BackToMenu});
        }
    }

    ImGui::End();
}

void DiddleDoodleDuel::renderLobbyUI() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f,
                                   viewport->Pos.y + viewport->Size.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Game Lobby", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImGui::Text("Waiting for players...");
    ImGui::Separator();
    ImGui::Spacing();

    if (multiplayerManager) {
        const auto& lobbyState = multiplayerManager->getLobbyState();
        
        // Show connection info
        if (multiplayerManager->isServer()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "You are hosting this game");
            ImGui::Text("Port: %d", serverPort);
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Connected to: %s:%d", serverAddress, serverPort);
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Show players
        ImGui::Text("Players (%zu/4):", lobbyState.players.size());
        ImGui::Spacing();
        
        for (const auto& player : lobbyState.players) {
            ImVec4 playerColor = ImVec4(
                player.color.r / 255.0f,
                player.color.g / 255.0f,
                player.color.b / 255.0f,
                1.0f
            );
            
            // Color indicator
            ImGui::ColorButton("##color", playerColor, ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
            ImGui::SameLine();
            
            // Player name and status
            ImGui::Text("%s", player.username.c_str());
            ImGui::SameLine();
            
            if (player.isReady) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Ready]");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[Not Ready]");
            }
            
            if (multiplayerManager->isServer() && player.playerId == 0) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "(Host)");
            }
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Game countdown
        if (lobbyState.gameStarting && lobbyState.countdown > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Game starting in: %u", lobbyState.countdown);
        }
        
        ImGui::Spacing();
        
        // Ready checkbox for client
        if (!multiplayerManager->isServer()) {
            static bool localPlayerReady = false;
            if (ImGui::Checkbox("Ready", &localPlayerReady)) {
                multiplayerManager->setPlayerReady(localPlayerReady);
            }
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Leave Lobby", ImVec2(150, 30))) {
        if (eventBus) {
            eventBus->dispatcher.trigger<MultiplayerEvent>(MultiplayerEvent{
                MultiplayerEvent::Type::Disconnect, ""
            });
        }
    }

    ImGui::End();
}

void DiddleDoodleDuel::executeUpdateOnActiveSystems(const float deltaTime) const {
    SimpleProfiler::getInstance().startTimer("SystemUpdate");
    

    if ((SystemsActivationSystem::shouldSystemRun(registry, "InputSystem"))) {
        SimpleProfiler::getInstance().startTimer("InputSystem");
        inputSystem->update();
        SimpleProfiler::getInstance().endTimer("InputSystem");
    }

    if ((SystemsActivationSystem::shouldSystemRun(registry, "PhysicsMovementSystem"))) {
        SimpleProfiler::getInstance().startTimer("PhysicsMovement");
        physicsMovementSystem->update(deltaTime);
        SimpleProfiler::getInstance().endTimer("PhysicsMovement");
    }

    if ((SystemsActivationSystem::shouldSystemRun(registry, "PhysicsCollisionSystem"))) {
        SimpleProfiler::getInstance().startTimer("PhysicsCollision");
        physicsCollisionSystem->update(deltaTime);
        SimpleProfiler::getInstance().endTimer("PhysicsCollision");
    }

    if ((SystemsActivationSystem::shouldSystemRun(registry, "PaintSystem"))) {
        SimpleProfiler::getInstance().startTimer("PaintSystem");
        paintSystem->update();
        SimpleProfiler::getInstance().endTimer("PaintSystem");
    }
    
    SimpleProfiler::getInstance().endTimer("SystemUpdate");
}

void DiddleDoodleDuel::executeRenderOnWorldSystems() const {

    if (SystemsActivationSystem::shouldSystemRun(registry, "PaintSystem")) {
        paintSystem->render();
    }

    if (SystemsActivationSystem::shouldSystemRun(registry, "ArrowRenderSystem")) {
        arrowRenderSystem->render();
    }

    if (SystemsActivationSystem::shouldSystemRun(registry, "UsernameRenderSystem")) {
        usernameRenderSystem->render();
    }
}

void DiddleDoodleDuel::handleInputEvents() const {
    if (!eventBus) {
        return;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::StartLocalGame});
    }
    if (IsKeyPressed(KEY_M)) {
        eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::BackToMenu});
    }
    if (IsKeyPressed(KEY_O)) {
        eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::StartOnlineGame});
    }
}

void DiddleDoodleDuel::renderUISystems(const SceneType currentScene) {

    uiSystem->render(title);

    if (SystemsActivationSystem::shouldSystemRun(registry, "ImGuiSystem")) {
        imguiSystem->beginFrame();

        switch (currentScene) {
            case SceneType::MainMenu:
                renderMainMenuUI();
                break;
            case SceneType::Game:
                imguiSystem->renderGameUI(title, GetFPS());
                imguiSystem->renderEcsDebug();
                break;
            case SceneType::NetworkingDemo:
                renderOnlineUI();
                break;
            case SceneType::Lobby:
                renderLobbyUI();
                break;
            case SceneType::NetworkedGame:
                imguiSystem->renderGameUI(title, GetFPS());
                imguiSystem->renderEcsDebug();
                break;
            default:
                break;
        }

        imguiSystem->endFrame();
    }
}

void DiddleDoodleDuel::renderDebugInfo(const SceneType currentScene) const {

    if (imguiSystem->isDebugWindowVisible() &&
        SystemsActivationSystem::shouldSystemRun(registry, "DebugRenderSystem")) {
        debugRenderSystem->render();
    }

    const std::string sceneText = "Current Scene: " + std::string(to_string(currentScene));
    DrawText(sceneText.c_str(), 10, 10, 20, WHITE);

    const std::string systemText =
        "ImGui System: " +
        std::string(SystemsActivationSystem::shouldSystemRun(registry, "ImGuiSystem") ? "Active"
                                                                                      : "Inactive");
    DrawText(systemText.c_str(), 10, 35, 20, WHITE);
}

void DiddleDoodleDuel::handleMultiplayerInput() const {
    if (!multiplayerManager || !multiplayerManager->isInGame()) {
        return;
    }
    
    bool rotateLeft = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    bool rotateRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    
    static bool lastRotateLeft = false;
    static bool lastRotateRight = false;
    if (rotateLeft != lastRotateLeft || rotateRight != lastRotateRight) {
        LOG_DEBUG_MSG("Input changed: Left=%d, Right=%d", rotateLeft, rotateRight);
        lastRotateLeft = rotateLeft;
        lastRotateRight = rotateRight;
    }
    
    multiplayerManager->sendPlayerInput(rotateLeft, rotateRight);
}
