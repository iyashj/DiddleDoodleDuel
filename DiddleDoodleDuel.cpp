#include "DiddleDoodleDuel.h"
#include <entt/entity/registry.hpp>

struct Input {
    bool rotateLeft { false };
    bool rotateRight { false };
};

struct Position {
    Vector2 pos{0.0F, 0.0F};
};

struct Velocity {
    Vector2 vel{0.0F, 0.0F};
    float rotationSpeed {180.0F};
};

struct Renderable {
    float radius{20.0F};
    Color color{WHITE};
};

struct Player : Renderable {
    float speed{30000.0F};
    float rotation{18000.0F};
};

DiddleDoodleDuel::DiddleDoodleDuel(engine::IRenderer& renderer) : Game(renderer) {
    SetTargetFPS(60);
}

DiddleDoodleDuel::~DiddleDoodleDuel() = default;

void DiddleDoodleDuel::onInitialize() {
    title = "Diddle Doodle Duel - ECS Base";
    const auto player = registry.create();

    registry.emplace<Position>(player, Vector2{640.0F, 360.0F});
    registry.emplace<Velocity>(player);
    registry.emplace<Renderable>(player, 25.0F, Color{0, 229, 255, 255});
    registry.emplace<Player>(player, 250.0F);
    registry.emplace<Input>(player, Input{ .rotateLeft = false, .rotateRight = false } );
}
void DiddleDoodleDuel::onUpdate(const float deltaTime) {

    const auto players = registry.view<Player, Input>();
    players.each([&](const auto&, Input& input) {
        input.rotateLeft = IsKeyDown(KEY_A);
        input.rotateRight = IsKeyDown(KEY_D);
    });

    const auto playerViews = registry.view<Player, Velocity, Input>();
    playerViews.each([&](Player& player, Velocity& velocity, const Input& input) {
        if (input.rotateLeft) {
            player.rotation -= velocity.rotationSpeed * deltaTime;
        }
        if (input.rotateRight) {
            player.rotation += velocity.rotationSpeed * deltaTime;
        }

        velocity.vel.x = cosf(player.rotation * DEG2RAD) * player.speed * deltaTime;
        velocity.vel.y = sinf(player.rotation * DEG2RAD) * player.speed * deltaTime;
    });

    for (const auto movementView = registry.view<Position, Velocity>();
        const auto entity: movementView) {
        auto& [pos] = movementView.get<Position>(entity);
        auto& [vel, rotationSpeed] = movementView.get<Velocity>(entity);

        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;

        if (registry.all_of<Player>(entity)) {
            pos.x = std::clamp(pos.x, 25.0F, 1255.0F);
            pos.y = std::clamp(pos.y, 25.0F, 695.0F);
        }
    }
}

void DiddleDoodleDuel::checkBoundsAgainstScreen() {
    const auto playerViews = registry.view<Player, Position>();
    playerViews.each([this](const Player& player, Position& position) {
        if (position.pos.x - player.radius < 0.0F) {
            position.pos.x = player.radius;
        }
        if (position.pos.x + player.radius > this->getRenderer().getWindowWidth()) {
           position.pos.x = this->getRenderer().getWindowWidth() - player.radius;
        }
        if (position.pos.y - player.radius < 0.0F) {
            position.pos.y = player.radius;
        }
        if (position.pos.y + player.radius > this->getRenderer().getWindowWidth()) {
            position.pos.y = this->getRenderer().getWindowWidth() - player.radius;
        }
    });
}


void DiddleDoodleDuel::onRender() {
    const auto renderViews = registry.view<Position, Renderable>();
    renderViews.each([&](const Position& position, const Renderable& renderable) {
        this->getRenderer().drawCircle(position.pos, renderable.radius, renderable.color);
    });

    this->getRenderer().drawText(title, Vector2 {20.0F, 20.0F }, 24, WHITE);
    this->getRenderer().drawText("Use A or D to update the rotation", Vector2 { 20.0F , 50.0F}, 18, LIGHTGRAY);
    DrawFPS(20, 100);
}