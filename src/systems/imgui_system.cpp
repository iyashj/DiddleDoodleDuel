#include "imgui_system.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <entt/entity/registry.hpp>
#include "components/position.h"
#include "components/velocity.h"
#include "components/collision_state.h"
#include "components/renderable.h"

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
    
    // UI controls that directly modify the config (no dirty flag needed)
    ImGui::SliderFloat("Brush Movement Speed", &gameConfig.brushMovementSpeed, 50.0f, 2000.0f);
    ImGui::SliderFloat("Brush Size", &gameConfig.brushSize, 10.0f, 50.0f);
    ImGui::SliderFloat("Collision Force", &gameConfig.collisionForceMultiplier, 0.5f, 10.0f);
    ImGui::SliderFloat("Bounce Duration (s)", &gameConfig.bounceDuration, 0.1f, 2.0f);
    ImGui::SliderFloat("Control During Bounce", &gameConfig.controlDuringBounceFactor, 0.0f, 1.0f);
    
    ImGui::Separator();
    ImGui::Text("Debug Options");
    ImGui::Checkbox("Show Collision Radius", &showDebugWindow);
    ImGui::SliderFloat("Debug Collision Radius", &gameConfig.debugCollisionRadius, 5.0f, 50.0f);

    ImGui::Checkbox("Show Debug Window", &showDebugWindow);
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

            auto view = registry.view<Position, Velocity, CollisionState>();
            for (auto entity : view) {
                const auto& pos = view.get<Position>(entity);
                const auto& vel = view.get<Velocity>(entity);
                const auto& col = view.get<CollisionState>(entity);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%u", (unsigned int)entity);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.1f", pos.position.x);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f", pos.position.y);

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.2f", vel.velocity.x);

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.2f", vel.velocity.y);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text(col.isInCollision ? "Yes" : "No");

                ImGui::TableSetColumnIndex(6);
                ImGui::Text("t=%.2f v=(%.1f,%.1f)", col.bounceTimer, col.bounceVelocity.x, col.bounceVelocity.y);
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

                auto view2 = registry.view<Renderable, Velocity>();
                for (auto entity : view2) {
                    const auto& ren = view2.get<Renderable>(entity);
                    const auto& vel = view2.get<Velocity>(entity);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%u", (unsigned int)entity);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.1f", ren.radius);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.1f deg", vel.rotation);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.1f", vel.speed);
                }
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();
}