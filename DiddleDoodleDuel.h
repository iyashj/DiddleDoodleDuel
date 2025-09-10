//
// Created by Yash Jain on 10.9.2025.
//

#ifndef DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#define DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#include "game/game.h"

#include <entt/entity/registry.hpp>

class DiddleDoodleDuel : public engine::Game {
public:
    explicit DiddleDoodleDuel(engine::IRenderer& renderer);
    ~DiddleDoodleDuel() override;
    void onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    entt::registry registry_;
    std::string title;

    void checkBoundsAgainstScreen();
};

#endif // DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
