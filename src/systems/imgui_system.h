#ifndef DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H
#define DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H
#include "game_config.h"

#include <string>

class ImGuiSystem {
public:
    ImGuiSystem(GameConfig& gameConfig);
    ~ImGuiSystem();

    bool initialize();
    void shutdown();
    
    void beginFrame() const;
    void endFrame() const;
    
    void renderGameUI(const std::string& title, int fps);
    void renderDebugWindow();
    
private:
    bool initialized = false;
    bool showDebugWindow = true;
    bool showDemoWindow = false;

    GameConfig& gameConfig;
};

#endif // DIDDLEDOODLEDUEL_IMGUI_SYSTEM_H