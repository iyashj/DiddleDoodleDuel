#ifndef DIDDLEDOODLEDUEL_GAME_PROTOCOL_H
#define DIDDLEDOODLEDUEL_GAME_PROTOCOL_H

#include <nlohmann/json.hpp>
#include <raylib.h>
#include <string>
#include <vector>

inline void to_json(nlohmann::json& j, const Vector2& v) {
    j = nlohmann::json{{"x", v.x}, {"y", v.y}};
}

inline void from_json(const nlohmann::json& j, Vector2& v) {
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
}

inline void to_json(nlohmann::json& j, const Color& c) {
    j = nlohmann::json{{"r", c.r}, {"g", c.g}, {"b", c.b}, {"a", c.a}};
}

inline void from_json(const nlohmann::json& j, Color& c) {
    j.at("r").get_to(c.r);
    j.at("g").get_to(c.g);
    j.at("b").get_to(c.b);
    j.at("a").get_to(c.a);
}

namespace network {

// Player information for lobby
struct PlayerInfo {
    uint32_t playerId;
    std::string username;
    Color color;
    bool isReady;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerInfo, playerId, username, color, isReady)
};

// Player input for game
struct PlayerInput {
    uint32_t playerId;
    bool rotateLeft;
    bool rotateRight;
    uint32_t inputSequence;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerInput, playerId, rotateLeft, rotateRight, inputSequence)
};

// Player state for synchronization
struct PlayerState {
    uint32_t playerId;
    Vector2 position;
    Vector2 velocity;
    float rotation;
    bool isInCollision;

    friend void to_json(nlohmann::json& j, const PlayerState& p) {
        j["playerId"] = p.playerId;
        j["position"] = p.position;
        j["velocity"] = p.velocity;
        j["rotation"] = p.rotation;
        j["isInCollision"] = p.isInCollision;
    }

    friend void from_json(const nlohmann::json& j, PlayerState& p) {
        j.at("playerId").get_to(p.playerId);
        p.position = j.at("position").get<Vector2>();
        p.velocity = j.at("velocity").get<Vector2>();
        j.at("rotation").get_to(p.rotation);
        j.at("isInCollision").get_to(p.isInCollision);
    }
};

// Complete game state
struct GameState {
    std::vector<PlayerState> players;
    uint32_t gameTime; // Game time in milliseconds
    uint32_t stateSequence; // For state synchronization
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameState, players, gameTime, stateSequence)
};

// Lobby state
struct LobbyState {
    std::vector<PlayerInfo> players;
    bool gameStarting;
    uint32_t countdown;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LobbyState, players, gameStarting, countdown)
};

// Game start information
struct GameStartInfo {
    std::vector<PlayerInfo> players;
    std::vector<Vector2> spawnPoints;
    uint32_t gameId;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameStartInfo, players, gameId)
};

// Paint stroke information
struct PaintStroke {
    uint32_t playerId;
    Vector2 position;
    float radius;
    Color color;

    friend void to_json(nlohmann::json& j, const PaintStroke& p) {
        j["playerId"] = p.playerId;
        j["position"] = p.position;
        j["radius"] = p.radius;
        j["color"] = p.color;
    }

    friend void from_json(const nlohmann::json& j, PaintStroke& p) {
        j.at("playerId").get_to(p.playerId);
        p.position = j.at("position").get<Vector2>();
        j.at("radius").get_to(p.radius);
        p.color = j.at("color").get<Color>();
    }
};

// Utility functions for serialization
template<typename T>
std::vector<uint8_t> serializeJson(const T& data) {
    nlohmann::json j = data;
    std::string jsonStr = j.dump();
    return std::vector<uint8_t>(jsonStr.begin(), jsonStr.end());
}

template<typename T>
T deserializeJson(const std::vector<uint8_t>& data) {
    std::string jsonStr(data.begin(), data.end());
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    return j.get<T>();
}

} // namespace network

#endif // DIDDLEDOODLEDUEL_GAME_PROTOCOL_H