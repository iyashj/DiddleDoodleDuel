#include "humble_engine.h"
#include "rendering/renderer.h"
#include "src/diddle_doodle_duel.h"
#include <string>

int main() {
    engine::Renderer renderer {};
    if (const auto init = renderer.initialize(1280, 720, "Diddle Doodle Duel");
        !init.has_value()) {
        return 1;
    }

    DiddleDoodleDuel game(renderer);
    game.run();

    return 0;
}