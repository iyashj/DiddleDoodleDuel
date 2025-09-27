#include "humble_engine.h"
#include "rendering/renderer.h"
#include "src/diddle_doodle_duel.h"
#include "logging/logger.h"
#include <string>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string windowTitle = "Diddle Doodle Duel - Server";
    
    int serverPort = 7777;
    
    if (argc >= 2) {
        serverPort = std::stoi(argv[1]);
    }
    
    LOG_INFO_MSG("Starting Diddle Doodle Duel Server on port %d", serverPort);
    
    engine::Renderer renderer {};
    if (const auto init = renderer.initialize(1280, 720, windowTitle);
        !init.has_value()) {
        return 1;
    }

    DiddleDoodleDuel game(renderer);
    game.run();

    return 0;
}