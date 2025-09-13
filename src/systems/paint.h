#ifndef DIDDLEDOODLEDUEL_PAINT_H
#define DIDDLEDOODLEDUEL_PAINT_H
#include "../components/drawing_structs.h"
#include "../components/movement_structs.h"
#include "../components/player.h"
#include "rendering/irenderer.h"
#include "resources/resource_manager.h"
#include <entt/entity/registry.hpp>
#include <raylib.h>

struct PaintSystem {
    explicit PaintSystem(engine::IWindowHandler& windowHandler) : windowHandler(windowHandler) {

        const auto width = windowHandler.getWindowWidth();
        const auto height = windowHandler.getWindowHeight();

        renderTexture = std::make_unique<RenderTexture2D>(LoadRenderTexture(width, height));
        shader = std::make_unique<Shader>(LoadShader(nullptr, "resources/shaders/watercolor.fs"));
        canvasPixel.assign(static_cast<size_t>(width) * static_cast<size_t>(height), WHITE);

        initialiseTexture();
    }

    void update(entt::registry& registry) {
        BeginTextureMode(*renderTexture);
        const auto view = registry.view<Position, Renderable>();
        view.each([&](const Position& position, const Renderable& renderable) {
            DrawCircle(static_cast<int>(position.pos.x), static_cast<int>(position.pos.y), renderable.radius, renderable.color);
            paintCPUBuffer(position.pos, renderable.radius, renderable.color);
        });
        EndTextureMode();
    }

    void render() const {
        drawTexture();
    }

private:
    const engine::IWindowHandler& windowHandler;
    std::unique_ptr<RenderTexture2D> renderTexture;
    std::unique_ptr<Shader> shader;
    std::vector<Color> canvasPixel;

    void initialiseTexture() const {
        BeginTextureMode(*renderTexture);
        ClearBackground(WHITE);
        EndTextureMode();
    }

    void drawTexture() const{
        BeginShaderMode(*shader);
        DrawTextureRec(renderTexture->texture,
            Rectangle{0,0, static_cast<float>(renderTexture->texture.width), static_cast<float>(renderTexture->texture.height)},
            Vector2{0.0f, 0.0f},
            WHITE);
        EndShaderMode();
    }
    void paintCPUBuffer(const Vector2& centre, float radius, const Color& color) {
        int w = windowHandler.getWindowWidth();
        int h = windowHandler.getWindowHeight();
        int xInInt = static_cast<int>(centre.x);
        int yInInt = static_cast<int>(centre.y);
        int radiusInInt = static_cast<int>(radius);
        for (int y = -radiusInInt; y <= radiusInInt; y++) {
            for (int x = -radiusInInt; x <= radiusInInt; x++) {
                if (x*x + y*y <= radiusInInt*radiusInInt) {
                    int draw_x = xInInt + x;
                    int draw_y = yInInt + y;
                    if (draw_x >= 0 && draw_x < w && draw_y >= 0 && draw_y < h) {
                        canvasPixel[draw_y * w + draw_x] = color;
                    }
                }
            }
        }
    }
};

#endif // DIDDLEDOODLEDUEL_PAINT_H
