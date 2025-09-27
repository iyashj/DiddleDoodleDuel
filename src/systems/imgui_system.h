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
    void renderConfigEditor();

    [[nodiscard]] bool isDebugWindowVisible() const { return showDebugWindow; }
    [[nodiscard]] bool isConfigEditorVisible() const { return showConfigEditor; }
    
private:
    bool initialized = false;
    bool showDebugWindow = true;
    bool showDemoWindow = false;
    bool showEcsWindow = true;
    bool showConfigEditor = true;

    GameConfig& gameConfig;
    entt::registry& registry;
    
    void saveConfigToFile();
    void loadConfigFromFile();
};

#endif // DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H