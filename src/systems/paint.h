#ifndef DIDDLEDOODLEDUEL_PAINT_H
#define DIDDLEDOODLEDUEL_PAINT_H
#include "components/renderable.h"
#include "components/scene_entity.h"
#include "game_config.h"
#include "logging/logger.h"
#include "network/multiplayer_manager.h"
#include "rendering/irenderer.h"
#include "resources/resource_manager.h"
#include "systems/scene_transition_system.h"
#include <entt/entity/registry.hpp>
#include <filesystem>
#include <raylib.h>
#include <raymath.h>
#include <unordered_map>
#include <vector>

struct PaintSystem {

    explicit PaintSystem(
        engine::IRenderer& renderer,
        GameConfig& config,
        entt::registry& registry,
        network::MultiplayerManager* multiplayerManager = nullptr)
    : config(config), registry(registry), renderer(renderer), multiplayerManager(multiplayerManager)
    {
        if (const auto result = engine::resources::loadTexture("textures/brush_base.png");
            result.has_value()) {
            this->brushBase = result.value();
            LOG_INFO_MSG("Paint system: Successfully loaded brush_base.png");
        } else {
            LOG_WARN_MSG("Paint system: Failed to load brush_base.png");
        }

        if (const auto result = engine::resources::loadTexture("textures/brush_mask.png");
            result.has_value()) {
            this->brushMask = result.value();
            LOG_INFO_MSG("Paint system: Successfully loaded brush_mask.png");
        } else {
            LOG_WARN_MSG("Paint system: Failed to load brush_mask.png");
        }

        const auto width = renderer.getWindowWidth();
        const auto height = renderer.getWindowHeight();

        renderTexture = std::make_unique<RenderTexture2D>(LoadRenderTexture(width, height));
        shader = std::make_unique<Shader>(LoadShader(nullptr, "shaders/watercolor.fs"));

        initialiseTexture();
    }

    void update() const {
        // Process received network paint strokes first
        if ((multiplayerManager != nullptr) && multiplayerManager->getConnectionStatus() == network::ConnectionStatus::Connected) {
            for (auto receivedStrokes = multiplayerManager->getAndClearReceivedPaintStrokes();
                 const auto& [playerId, position, radius, color] : receivedStrokes) {
                // Only apply paint strokes from remote players (not our own)
                if (playerId != multiplayerManager->getLocalPlayerId()) {
                    LOG_DEBUG_MSG("Paint system: Applying remote paint stroke from player %u at (%.2f, %.2f) with color (%d,%d,%d,%d)", 
                                  playerId, position.x, position.y,
                                  static_cast<int>(color.r), static_cast<int>(color.g),
                                  static_cast<int>(color.b), static_cast<int>(color.a));
                              
                    BeginTextureMode(*renderTexture);
                    DrawCircle(static_cast<int>(position.x), static_cast<int>(position.y), radius, color);
                    EndTextureMode();
                } else {
                    LOG_DEBUG_MSG("Paint system: Ignoring own paint stroke echo from server");
                }
            }
        }

        const auto currentScene = SceneTransitionSystem::getCurrentScene(registry);
        const auto view = registry.view<Position, Renderable, SceneEntity>();
        
        static int updateDebugCounter = 0;
        bool shouldDebug = (updateDebugCounter++ % 120 == 0); // Debug every 2 seconds
        
        if (shouldDebug) {
            LOG_DEBUG_MSG("Paint system update: Checking entities with Position+Renderable+SceneEntity");
        }

        for (auto entity : view) {
            const auto& pos = view.get<Position>(entity);
            const auto& renderable = view.get<Renderable>(entity);
            const auto& sceneEntity = view.get<SceneEntity>(entity);
            
            // Only update entities that belong to the current scene
            if (sceneEntity.belongsToScene != currentScene) {
                continue;
            }

            // Use config.brushSize instead of radius for consistent sizing
            drawBrush(pos.position, config.brushSize, renderable.color);

            // Store last positions to only paint when actually moving
            static std::unordered_map<entt::entity, Vector2> lastPositions;

            if (auto it = lastPositions.find(entity);
                it == lastPositions.end() ||
                Vector2Distance(it->second, pos.position) > renderable.radius * 0.1f) {
                lastPositions[entity] = pos.position;

                if (shouldDebug) {
                    LOG_DEBUG_MSG("Paint system: Painting entity %u at (%.2f, %.2f)", 
                                  static_cast<uint32_t>(entity), pos.position.x, pos.position.y);
                }

                // Apply paint locally
                BeginTextureMode(*renderTexture);
                DrawCircle(static_cast<int>(pos.position.x), static_cast<int>(pos.position.y), renderable.radius, renderable.color);
                EndTextureMode();

                // Send paint stroke over network if this is the local player
                if (multiplayerManager && 
                    multiplayerManager->getConnectionStatus() == network::ConnectionStatus::Connected &&
                    multiplayerManager->isLocalPlayerEntity(entity)) {
                    
                    LOG_DEBUG_MSG("Paint system: Sending paint stroke for local player entity %u with color (%d,%d,%d,%d)", 
                                  static_cast<uint32_t>(entity), static_cast<int>(renderable.color.r), 
                                  static_cast<int>(renderable.color.g), static_cast<int>(renderable.color.b), 
                                  static_cast<int>(renderable.color.a));
                    multiplayerManager->sendPaintStroke(
                        multiplayerManager->getLocalPlayerId(),
                        pos.position,
                        renderable.radius,
                        renderable.color
                    );
                }
            }
        }
    }

    void render() const {
        drawTexture();
        drawBrush(registry, renderer, config);
    }

private:
    std::unique_ptr<RenderTexture2D> renderTexture;
    std::unique_ptr<Shader> shader;
    Texture2D brushBase{};
    Texture2D brushMask{};
    const GameConfig& config;
    entt::registry& registry;
    engine::IRenderer& renderer;
    network::MultiplayerManager* multiplayerManager;

    void initialiseTexture() const {
        BeginTextureMode(*renderTexture);
        ClearBackground(WHITE);
        EndTextureMode();
    }

    void drawTexture() const{
        BeginShaderMode(*shader);
        DrawTextureRec(renderTexture->texture,
            Rectangle{0,0, static_cast<float>(renderTexture->texture.width), static_cast<float>(-renderTexture->texture.height)},
            Vector2{0.0F, 0.0F},
            WHITE);
        EndShaderMode();
    }

    void drawBrush(const entt::registry& registry, engine::IDrawHandler& drawHandler, const GameConfig& config) const {
        const auto currentScene = SceneTransitionSystem::getCurrentScene(registry);
        
        // Debug: Check if textures are loaded
        if (brushBase.id == 0 || brushMask.id == 0) {
            static bool textureWarningShown = false;
            if (!textureWarningShown) {
                LOG_WARN_MSG("Paint system: Warning - brush textures not loaded (base: %u, mask: %u)", 
                             brushBase.id, brushMask.id);
                textureWarningShown = true;
            }
            return;
        }
        
        static int debugCounter = 0;
        const bool shouldDebug = (debugCounter++ % 60 == 0); // Debug every second at 60 FPS
        
        for (const auto view = registry.view<const Position, const Renderable, const SceneEntity>();
             const auto& entity : view) {
            const auto& [pos] = view.get<const Position>(entity);
            const auto& [radius, color] = view.get<const Renderable>(entity);

            // Only render entities that belong to the current scene
            if (const auto& [belongsToScene, persistent] = view.get<const SceneEntity>(entity);
                belongsToScene != currentScene) {
                continue;
            }
            
            if (shouldDebug) {
                LOG_DEBUG_MSG("Paint system: Drawing brush for entity %u at (%.2f, %.2f)", 
                              static_cast<uint32_t>(entity), pos.x, pos.y);
            }

            const float brushSize = config.brushSize * 2.0F;

            const Rectangle destinationRect = {
                pos.x,
                pos.y,
                brushSize,
                brushSize
            };

            Vector2 origin = {brushSize / 2.0F, brushSize / 2.0F};
            constexpr float noRotation = 0.0F;

            drawHandler.drawTexture(
                brushBase, {0,0, static_cast<float>(brushBase.width), static_cast<float>(brushBase.height)},
                destinationRect,
                origin,
                noRotation,
                WHITE);

            drawHandler.drawTexture(
                brushMask, {0, 0, static_cast<float>(brushMask.width), static_cast<float>(brushMask.height)},
                destinationRect,
                origin,
                noRotation,
                color);
        }
    }

    void drawBrush(const Vector2& pos, float size, const Color& color) const {
        // This method can be used for immediate drawing effects
        // For now, it just draws to the render texture
        BeginTextureMode(*renderTexture);
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), size, color);
        EndTextureMode();
    }
};

#endif // DIDDLEDOODLEDUEL_PAINT_H
