#ifndef DIDDLEDOODLEDUEL_SCENE_TYPE_H
#define DIDDLEDOODLEDUEL_SCENE_TYPE_H

enum class SceneType {
    MainMenu,
    PlayerProfile,
    Game,
    Paused,
    GameOver,
    NetworkingDemo,
    Lobby,
    NetworkedGame
};

inline const char* to_string(const SceneType type) {
    switch (type) {
        case SceneType::MainMenu: return "MainMenu";
        case SceneType::PlayerProfile: return "PlayerProfile";
        case SceneType::Game: return "Game";
        case SceneType::Paused: return "Paused";
        case SceneType::GameOver: return "GameOver";
        case SceneType::NetworkingDemo: return "NetworkingDemo";
        case SceneType::Lobby: return "Lobby";
        case SceneType::NetworkedGame: return "NetworkedGame";
        default: return "unknown";
    }
}

#endif // DIDDLEDOODLEDUEL_SCENE_TYPE_H
