#include "humble_engine.h"
#include "rendering/renderer.h"
#include "src/diddle_doodle_duel.h"
#include <string>

int main(int argc, char* argv[]) {
    std::string windowTitle = "Diddle Doodle Duel - Client";
    
    std::string serverHost = "127.0.0.1";
    int serverPort = 7777;
    
    if (argc >= 3) {
        serverHost = argv[1];
        serverPort = std::stoi(argv[2]);
    }
    
    engine::Renderer renderer {};
    if (const auto init = renderer.initialize(1280, 720, windowTitle);
        !init.has_value()) {
        return 1;
    }

    DiddleDoodleDuel game(renderer);
    game.run();

    return 0;
}