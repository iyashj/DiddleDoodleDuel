#ifndef DIDDLEDOODLEDUEL_MULTIPLAYER_MANAGER_H
#define DIDDLEDOODLEDUEL_MULTIPLAYER_MANAGER_H

#include "network_interface.h"
#include "game_protocol.h"
#include "game_config.h"
#include <entt/entity/registry.hpp>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace network {

enum class MultiplayerState {
    Disconnected,
    Connecting,
    InLobby,
    InGame
};

class MultiplayerManager {
public:
    explicit MultiplayerManager(entt::registry& registry, GameConfig& gameConfig);
    ~MultiplayerManager();
    
    // Connection management
    bool startServer(uint16_t port = 7777);
    bool connectToServer(const std::string& host, uint16_t port = 7777);
    void disconnect();
    
    // Lobby management
    void setPlayerInfo(const std::string& username, Color color);
    void setPlayerReady(bool ready);
    
    // Game management
    void sendPlayerInput(bool rotateLeft, bool rotateRight);
    void sendPaintStroke(uint32_t playerId, Vector2 position, float radius, Color color);
    std::vector<PaintStroke> getAndClearReceivedPaintStrokes();
    void update(float deltaTime);
    
    // State queries
    MultiplayerState getState() const { return currentState; }
    const LobbyState& getLobbyState() const { return lobbyState; }
    ConnectionStatus getConnectionStatus() const;
    bool isServer() const;
    bool isInGame() const { return currentState == MultiplayerState::InGame; }
    uint32_t getLocalPlayerId() const { return localPlayerInfo.playerId; }
    bool isLocalPlayerEntity(entt::entity entity) const;
    
    // Event callbacks
    using StateChangeHandler = std::function<void(MultiplayerState)>;
    using LobbyUpdateHandler = std::function<void(const LobbyState&)>;
    using GameStartHandler = std::function<void(const GameStartInfo&)>;
    
    void setStateChangeHandler(StateChangeHandler handler) { stateChangeHandler = std::move(handler); }
    void setLobbyUpdateHandler(LobbyUpdateHandler handler) { lobbyUpdateHandler = std::move(handler); }
    void setGameStartHandler(GameStartHandler handler) { gameStartHandler = std::move(handler); }

private:
    entt::registry& registry;
    GameConfig& gameConfig;
    std::unique_ptr<INetworkInterface> network;
    
    MultiplayerState currentState = MultiplayerState::Disconnected;
    LobbyState lobbyState;
    PlayerInfo localPlayerInfo;
    
    // Game state
    std::unordered_map<uint32_t, entt::entity> playerEntities;
    uint32_t inputSequence = 0;
    uint32_t lastStateSequence = 0;
    
    // Server-specific
    std::chrono::steady_clock::time_point gameStartTime;
    std::chrono::steady_clock::time_point countdownStartTime;
    bool countdownActive = false;
    
    // Client-specific
    std::vector<PlayerInput> pendingInputs; // For client prediction
    std::vector<PaintStroke> receivedPaintStrokes; // Store incoming paint strokes
    
    // Event handlers
    StateChangeHandler stateChangeHandler;
    LobbyUpdateHandler lobbyUpdateHandler;
    GameStartHandler gameStartHandler;
    
    // Network event handlers
    void onNetworkMessage(const NetworkMessage& message);
    void onConnectionStatusChanged(ConnectionStatus status);
    
    // Message handlers
    void handlePlayerJoin(const NetworkMessage& message);
    void handlePlayerLeave(const NetworkMessage& message);
    void handlePlayerInfo(const NetworkMessage& message);
    void handlePlayerReady(const NetworkMessage& message);
    void handleGameStart(const NetworkMessage& message);
    void handleLobbyState(const NetworkMessage& message);
    void handlePlayerInput(const NetworkMessage& message);
    void handleGameState(const NetworkMessage& message);
    void handlePaintStroke(const NetworkMessage& message);
    
    // Server logic
    void updateServerLobby(float deltaTime);
    void updateServerGame(float deltaTime);
    void checkGameStart();
    void startCountdown();
    void broadcastLobbyState();
    void broadcastGameState();
    void createPlayerEntity(const PlayerInfo& playerInfo, const Vector2& spawnPos);
    
    // Client logic
    void updateClientGame(float deltaTime);
    void applyClientPrediction();
    void reconcileWithServerState(const GameState& serverState);
    
    // Utility
    void setState(MultiplayerState newState);
    PlayerInfo* findPlayerInfo(uint32_t playerId);
    Vector2 getSpawnPosition(size_t playerIndex);
    std::vector<Vector2> getSpawnPositions();
};

} // namespace network

#endif // DIDDLEDOODLEDUEL_MULTIPLAYER_MANAGER_H