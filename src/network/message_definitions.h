#ifndef DIDDLEDOODLEDUEL_MESSAGE_DEFINITIONS_H
#define DIDDLEDOODLEDUEL_MESSAGE_DEFINITIONS_H
#include <vector>

enum class MessageType : uint8_t {
    // Connection messages
    PlayerJoin = 1,
    PlayerLeave = 2,

    // Lobby messages
    PlayerInfo = 10,
    PlayerReady = 11,
    GameStart = 12,
    LobbyState = 13,

    // Game messages
    PlayerInput = 20,
    GameState = 21,
    PlayerUpdate = 22,
    PaintStroke = 23,

    // System messages
    Ping = 30,
    Pong = 31
};

struct NetworkMessage {
    MessageType type;
    uint32_t playerId;
    std::vector<uint8_t> data;

    explicit NetworkMessage(const MessageType type, const uint32_t playerId = 0)
        : type(type), playerId(playerId) {}
};

#endif // DIDDLEDOODLEDUEL_MESSAGE_DEFINITIONS_H
