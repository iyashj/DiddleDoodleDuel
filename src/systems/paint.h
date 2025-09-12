#ifndef DIDDLEDOODLEDUEL_PAINT_H
#define DIDDLEDOODLEDUEL_PAINT_H
#include "../components/movement_structs.h"
#include "../components/player.h"
#include "../components/drawing_structs.h"
#include "rendering/irenderer.h"
#include <entt/entity/registry.hpp>
#include <raylib.h>
#include <vector>

struct PaintSystem {

    PaintSystem(
        std::vector<Color>& canvasPixel,
        engine::IWindowHandler& windowHandler) : canvasPixel(canvasPixel), windowHandler(windowHandler){}

    std::vector<Color>& canvasPixel;
    engine::IWindowHandler& windowHandler;

    void update(entt::registry& registry) const {
        const auto view = registry.view<Player, Position, Renderable>();
        view.each([&](const Player&, const Position& position, const Renderable& renderable) {
            paint(position.pos, renderable.radius, renderable.color, canvasPixel);
        });
    }

    void paint(const Vector2 centre,
        const float radius,
        const Color& color,
        std::vector<Color>& canvas) const {
        const int xInInt = static_cast<int>(centre.x);
        const int yInInt = static_cast<int>(centre.y);
        const int radiusInInt = static_cast<int>(radius);
        for (int y = -radiusInInt; y <= radiusInInt; y++) {
            for (int x = -radiusInInt; x <= radiusInInt; x++) {
                if (x*x + y*y <= radiusInInt*radiusInInt) {
                    const int draw_x = x + x;
                    const int draw_y = y + y;
                    if (draw_x >= 0 && draw_x < windowHandler.getWindowWidth() &&
                        draw_y >= 0 && draw_y < windowHandler.getWindowHeight())
                    {
                        canvas[draw_y * windowHandler.getWindowWidth() + draw_x] = color;
                    }
                }
            }
        }
    }

    void render(entt::registry& registry, engine::IDrawHandler& drawHandler) {
        const auto renderViews = registry.view<Position, Renderable>();
        renderViews.each([&](const Position& position, const Renderable& renderable) {
            drawHandler.drawCircle(position.pos, renderable.radius, renderable.color);
        });
    }
};

#endif // DIDDLEDOODLEDUEL_PAINT_H
