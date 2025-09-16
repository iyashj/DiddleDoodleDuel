#ifndef DIDDLEDOODLEDUEL_PAINT_H
#define DIDDLEDOODLEDUEL_PAINT_H
#include "components/renderable.h"
#include "rendering/irenderer.h"
#include "resources/resource_manager.h"
#include "game_config.h"
#include <entt/entity/registry.hpp>
#include <raylib.h>
#include <raymath.h>
#include <unordered_map>

struct PaintSystem {

    explicit PaintSystem(engine::IRenderer& renderer, GameConfig& config, entt::registry& registry)
    : config(config), registry(registry), renderer(renderer)
    {
        if (const auto result = engine::resources::loadTexture("resources/textures/brush_base.png");
            result.has_value()) {
            this->brushBase = result.value();
        }

        if (const auto result = engine::resources::loadTexture("resources/textures/brush_mask.png");
            result.has_value()) {
            this->brushMask = result.value();
        }

        const auto width = renderer.getWindowWidth();
        const auto height = renderer.getWindowHeight();

        renderTexture = std::make_unique<RenderTexture2D>(LoadRenderTexture(width, height));
        shader = std::make_unique<Shader>(LoadShader(nullptr, "resources/shaders/watercolor.fs"));

        initialiseTexture();
    }

    void update() const {
        const auto view = registry.view<Position, Renderable>();
        bool needsPainting = false;

        for (auto entity : view) {
            const auto& [pos] = view.get<Position>(entity);
            const auto& [radius, color] = view.get<Renderable>(entity);
            
            // Use config.brushSize instead of radius for consistent sizing
            drawBrush(pos, config.brushSize, color);
            
            // Store last positions to only paint when actually moving
            static std::unordered_map<entt::entity, Vector2> lastPositions;
            
            auto it = lastPositions.find(entity);
            if (it == lastPositions.end() || 
                Vector2Distance(it->second, pos) > radius * 0.1f) {
                lastPositions[entity] = pos;
                needsPainting = true;

                BeginTextureMode(*renderTexture);
                DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), radius, color);
                EndTextureMode();
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
        for (const auto view = registry.view<const Position, const Renderable>();
             const auto& entity : view) {
            const auto& [pos] = view.get<const Position>(entity);
            const auto& [radius, color] = view.get<const Renderable>(entity);

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
