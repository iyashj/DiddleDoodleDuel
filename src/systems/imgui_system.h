#ifndef DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H
#define DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H
#include "game_config.h"

#include <entt/entity/registry.hpp>
#include <string>

class ImGuiSystem {
public:
    ImGuiSystem(entt::registry& registry, GameConfig& gameConfig);
    ~ImGuiSystem();

    bool initialize();
    void shutdown();
    
    void beginFrame() const;
    void endFrame() const;
    
    void renderGameUI(const std::string& title, int fps);
    void renderDebugWindow();
    void renderEcsDebug();

    [[nodiscard]] bool isDebugWindowVisible() const { return showDebugWindow; }
    
private:
    bool initialized = false;
    bool showDebugWindow = true;
    bool showDemoWindow = false;
    bool showEcsWindow = true;

    GameConfig& gameConfig;
    entt::registry& registry;
};

#endif // DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H