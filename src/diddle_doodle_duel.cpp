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
            SceneTransitionSystem::requestTransition(registry, SceneType::MainMenu);
            break;
    }
}

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer) : Game(renderer) {
    SetTargetFPS(60);

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
    paintSystem =
        std::make_unique<PaintSystem>(PaintSystem(this->getRenderer(), gameConfig, registry));
    physicsMovementSystem =
        std::make_unique<PhysicsMovementSystem>(PhysicsMovementSystem(registry, gameConfig));
    inputSystem = std::make_unique<InputSystem>(InputSystem(registry));
    uiSystem = std::make_unique<UISystem>(this->getRenderer());
    physicsCollisionSystem =
        std::make_unique<PhysicsCollisionSystem>(PhysicsCollisionSystem(registry, gameConfig));
    debugRenderSystem = std::make_unique<DebugRenderSystem>(registry, gameConfig);
    arrowRenderSystem = std::make_unique<ArrowRenderSystem>(registry, this->getRenderer());
}

DiddleDoodleDuel::~DiddleDoodleDuel() {
    LOG_DEBUG_MSG("Cleaning up game resources...");
    
    // Print final performance report
    SimpleProfiler::getInstance().printResults();
    
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
    
    // Manual FPS calculation to bypass display limitations
    static float frameCount = 0;
    static float timeAccumulator = 0;
    frameCount++;
    timeAccumulator += deltaTime;
    
    if (timeAccumulator >= 1.0f) {
        std::cout << "TRUE FPS: " << frameCount << " (Frame time: " << (timeAccumulator/frameCount)*1000 << "ms)" << std::endl;
        frameCount = 0;
        timeAccumulator = 0;
    }
    
    // Print performance stats every 5 seconds
    static float timeSinceLastProfile = 0.0f;
    timeSinceLastProfile += deltaTime;
    if (timeSinceLastProfile >= 5.0f) {
        SimpleProfiler::getInstance().printResults();
        SimpleProfiler::getInstance().reset();
        timeSinceLastProfile = 0.0f;
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
        std::cout << "DEBUG: Rendering Main Menu UI!" << std::endl;
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

void DiddleDoodleDuel::renderOnlineUI() const {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f,
                                   viewport->Pos.y + viewport->Size.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);

    ImGui::Begin("Online Game", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImGui::Text("Online Multiplayer");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Work in Progress");
    ImGui::Spacing();
    ImGui::Text("Online multiplayer functionality");
    ImGui::Text("is currently under development.");
    ImGui::Spacing();
    ImGui::Text("Stay tuned for updates!");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Back to Main Menu", ImVec2(200, 30))) {
        if (eventBus) {
            eventBus->dispatcher.trigger<MenuEvent>(MenuEvent{MenuEvent::Type::BackToMenu});
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

void DiddleDoodleDuel::renderUISystems(const SceneType currentScene) const {

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
