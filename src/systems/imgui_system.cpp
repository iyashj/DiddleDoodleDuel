#include "imgui_system.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

ImGuiSystem::ImGuiSystem(GameConfig& gameConfig) : gameConfig(gameConfig) {
}

ImGuiSystem::~ImGuiSystem() {
    if (initialized) {
        shutdown();
    }
}

bool ImGuiSystem::initialize() {
    if (initialized) {
        return true;
    }

    GLFWwindow* window = glfwGetCurrentContext();
    if (window == nullptr) {
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    initialized = true;
    return true;
}

void ImGuiSystem::shutdown() {
    if (initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        initialized = false;
    }
}

void ImGuiSystem::beginFrame() const {
    if (!initialized) {
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiSystem::endFrame() const {
    if (!initialized) {
        return;
    }
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiSystem::renderGameUI(const std::string& title, int fps) {
    if (!initialized) {
        return;
    }

    ImGui::Begin("Game Controls");
    
    ImGui::Text("Game: %s", title.c_str());
    ImGui::Text("FPS: %d", fps);
    ImGui::Separator();
    
    // UI controls that directly modify the config (no dirty flag needed)
    ImGui::SliderFloat("Brush Movement Speed", &gameConfig.brushMovementSpeed, 50.0f, 2000.0f);
    ImGui::SliderFloat("Brush Size", &gameConfig.brushSize, 10.0f, 50.0f);
    ImGui::SliderFloat("Collision Force", &gameConfig.collisionForceMultiplier, 0.5f, 5.0f);
    
    ImGui::Checkbox("Show Debug Window", &showDebugWindow);
    ImGui::Checkbox("Show ImGui Demo", &showDemoWindow);
    
    ImGui::End();
    
    // Show debug window if enabled
    if (showDebugWindow) {
        renderDebugWindow();
    }
    
    // Show ImGui demo window if enabled
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void ImGuiSystem::renderDebugWindow() {
    ImGui::Begin("Debug Info", &showDebugWindow);
    
    ImGui::Text("Memory Usage:");
    
    ImGui::Separator();
    
    ImGui::Text("Performance:");
    ImGui::Text("  Target FPS: 60");
    ImGui::Text("  Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    
    ImGui::Separator();
    
    ImGui::Text("Controls:");
    ImGui::BulletText("A/D - Player 1 rotation");
    ImGui::BulletText("Left/Right - Player 2 rotation");
    
    ImGui::End();
}