#ifndef DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#define DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
#include "game/game.h"

#include <entt/entity/registry.hpp>

class diddle_doodle_duel : public engine::Game {
public:
    explicit diddle_doodle_duel(engine::IRenderer& renderer);
    ~diddle_doodle_duel() override;
    void onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    entt::registry registry;
    std::string title;

    void checkBoundsAgainstScreen();
};

#endif // DIDDLEDOODLEDUEL_DIDDLEDOODLEDUEL_H
