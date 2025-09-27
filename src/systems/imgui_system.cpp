#include "imgui_system.h"
#include "components/collision_state.h"
#include "components/input_action.h"
#include "components/input_mapping.h"
#include "components/position.h"
#include "components/renderable.h"
#include "components/velocity.h"
#include "core/game_config_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <entt/entity/registry.hpp>

ImGuiSystem::ImGuiSystem(entt::registry& registry, GameConfig& gameConfig) : gameConfig(gameConfig), registry(registry) {
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

    ImGui::Separator();
    ImGui::Text("Collision Physics");
    ImGui::SliderFloat("Restitution", &gameConfig.restitution, 0.0f, 1.0f);
    ImGui::SliderFloat("Collision Damping", &gameConfig.collisionDamping, 0.1f, 1.0f);
    ImGui::SliderFloat("Separation Force", &gameConfig.separationForce, 50.0f, 300.0f);
    ImGui::SliderFloat("Brush Size", &gameConfig.brushSize, 10.0f, 50.0f);
    
    ImGui::Separator();
    ImGui::Text("Debug Options");
    ImGui::Checkbox("Show Collision Radius", &showDebugWindow);
    ImGui::SliderFloat("Debug Collision Radius", &gameConfig.debugCollisionRadius, 5.0f, 50.0f);

    ImGui::Checkbox("Show Debug Window", &showDebugWindow);
    ImGui::Checkbox("Show Config Editor", &showConfigEditor);
    ImGui::Checkbox("Show ECS State", &showEcsWindow);
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

void ImGuiSystem::renderEcsDebug() {
    if (!initialized || !showEcsWindow) return;

    if (ImGui::Begin("ECS State", &showEcsWindow)) {
        // Table of key components
        if (ImGui::BeginTable("ecs_table", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Entity");
            ImGui::TableSetupColumn("PosX");
            ImGui::TableSetupColumn("PosY");
            ImGui::TableSetupColumn("VelX");
            ImGui::TableSetupColumn("VelY");
            ImGui::TableSetupColumn("Colliding");
            ImGui::TableSetupColumn("Bounce t/V");
            ImGui::TableHeadersRow();

            for (const auto view = registry.view<Position, Velocity, CollisionState>();
                 auto entity : view) {
                const auto& [position] = view.get<Position>(entity);
                const auto& vel = view.get<Velocity>(entity);
                const auto& [isInCollision, bounceTimer, bounceVelocity] = view.get<CollisionState>(entity);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%u", static_cast<unsigned int>(entity));

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f", position.x);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f", position.y);

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.2f", vel.velocity.x);

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.2f", vel.velocity.y);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text(isInCollision ? "Yes" : "No");

                ImGui::TableSetColumnIndex(6);
                ImGui::Text("t=%.2f v=(%.1f,%.1f)", bounceTimer, bounceVelocity.x, bounceVelocity.y);
            }
            ImGui::EndTable();
        }

        // Optional: show renderable and rotation
        if (ImGui::CollapsingHeader("Renderable/Rotation")) {
            if (ImGui::BeginTable("ecs_table_extra", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Entity");
                ImGui::TableSetupColumn("Radius");
                ImGui::TableSetupColumn("Rotation");
                ImGui::TableSetupColumn("Speed");
                ImGui::TableHeadersRow();

                for (const auto view2 = registry.view<Renderable, Velocity>(); auto entity : view2) {
                    const auto& [radius, color] = view2.get<Renderable>(entity);
                    const auto& vel = view2.get<Velocity>(entity);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%u", static_cast<unsigned int>(entity));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.1f", radius);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.1f deg", vel.rotation);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.1f", vel.speed);
                }
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Input")) {
            if (ImGui::BeginTable("ecs_inputs", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Entity");
                ImGui::TableSetupColumn("Rotate Left");
                ImGui::TableSetupColumn("Rotate Right");
                ImGui::TableHeadersRow();

                for (const auto view3 = registry.view<InputAction>(); auto entity : view3) {
                    const auto& [rotateLeft, rotateRight] = view3.get<InputAction>(entity);

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%u", static_cast<unsigned int>(entity));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", rotateLeft);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", rotateRight);
                }
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();
}

void ImGuiSystem::renderConfigEditor() {
    if (!initialized || !showConfigEditor) {
        return;
    }

    ImGui::Begin("Game Configuration", &showConfigEditor);
    
    ImGui::Text("Runtime Configuration Editor");
    ImGui::Separator();
    
    // Brush Settings
    if (ImGui::CollapsingHeader("Brush Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Brush Size", &gameConfig.brushSize, 10.0f, 100.0f, "%.1f");
        ImGui::SliderFloat("Brush Movement Speed", &gameConfig.brushMovementSpeed, 100.0f, 10000.0f, "%.0f");
        ImGui::SliderFloat("Collision Force Multiplier", &gameConfig.collisionForceMultiplier, 0.5f, 10.0f, "%.2f");
    }
    
    // Physics Settings
    if (ImGui::CollapsingHeader("Physics Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Restitution", &gameConfig.restitution, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Collision Damping", &gameConfig.collisionDamping, 0.1f, 1.0f, "%.2f");
        ImGui::SliderFloat("Separation Force", &gameConfig.separationForce, 50.0f, 500.0f, "%.0f");
        ImGui::SliderFloat("Bounce Duration", &gameConfig.bounceDuration, 0.1f, 2.0f, "%.2f");
        ImGui::SliderFloat("Control During Bounce Factor", &gameConfig.controlDuringBounceFactor, 0.0f, 1.0f, "%.2f");
    }
    
    // Debug Settings
    if (ImGui::CollapsingHeader("Debug Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Debug Collision Radius", &gameConfig.debugCollisionRadius, 5.0f, 100.0f, "%.1f");
        ImGui::Checkbox("Enable Profiler", &gameConfig.enableProfiler);
        ImGui::Checkbox("Enable FPS Counter", &gameConfig.enableFpsCounter);
    }
    
    ImGui::Separator();
    
    // Action buttons
    if (ImGui::Button("Save to File")) {
        saveConfigToFile();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load from File")) {
        loadConfigFromFile();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset to Defaults")) {
        gameConfig = GameConfig{}; // Reset to default values
    }
    
    ImGui::End();
}

void ImGuiSystem::saveConfigToFile() {
    const auto configPath = GameConfigManager::getConfigPath();
    if (GameConfigManager::saveToJson(configPath, gameConfig)) {
        // Could show a success notification here
    }
}

void ImGuiSystem::loadConfigFromFile() {
    const auto configPath = GameConfigManager::getConfigPath();
    if (GameConfigManager::loadFromJson(configPath, gameConfig)) {
        // Could show a success notification here
    }
}