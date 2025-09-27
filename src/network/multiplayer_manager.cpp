#include "multiplayer_manager.h"
#include "enet_network.h"
#include "components/position.h"
#include "components/velocity.h"
#include "components/renderable.h"
#include "components/input_action.h"
#include "components/input_mapping.h"
#include "components/collision_state.h"
#include "components/username.h"
#include "systems/entity_lifecycle_system.h"
#include "systems/scene_transition_system.h"
#include "logging/logger.h"
#include <iostream>

namespace network {

MultiplayerManager::MultiplayerManager(entt::registry& registry, GameConfig& gameConfig)
    : registry(registry), gameConfig(gameConfig) {
    network = createNetworkInterface();
    
    network->setMessageHandler([this](const NetworkMessage& msg) {
        onNetworkMessage(msg);
    });
    
    network->setConnectionStatusHandler([this](ConnectionStatus status) {
        onConnectionStatusChanged(status);
    });
    
    // Default player info
    localPlayerInfo.username = "Player";
    localPlayerInfo.color = BLUE;
    localPlayerInfo.isReady = false;
}

MultiplayerManager::~MultiplayerManager() {
    disconnect();
}

bool MultiplayerManager::startServer(uint16_t port) {
    if (currentState != MultiplayerState::Disconnected) {
        return false;
    }
    
    if (network->startServer(port)) {
        setState(MultiplayerState::InLobby);
        
        // Server is always "ready" and in the lobby
        localPlayerInfo.playerId = 0;
        localPlayerInfo.isReady = true;
        lobbyState.players.push_back(localPlayerInfo);
        
        LOG_INFO_MSG("Server started on port %d", port);
        return true;
    }
    
    return false;
}

bool MultiplayerManager::connectToServer(const std::string& host, uint16_t port) {
    if (currentState != MultiplayerState::Disconnected) {
        return false;
    }
    
    setState(MultiplayerState::Connecting);
    
    if (network->connectToServer(host, port)) {
        LOG_INFO_MSG("Attempting to connect to %s:%d", host.c_str(), port);
        return true;
    } else {
        setState(MultiplayerState::Disconnected);
        return false;
    }
}

void MultiplayerManager::disconnect() {
    network->disconnect();
    setState(MultiplayerState::Disconnected);
    
    // Clean up game entities
    for (const auto& [playerId, entity] : playerEntities) {
        if (registry.valid(entity)) {
            registry.destroy(entity);
        }
    }
    playerEntities.clear();
    
    // Reset state
    lobbyState = LobbyState{};
    inputSequence = 0;
    lastStateSequence = 0;
    countdownActive = false;
    pendingInputs.clear();
}

void MultiplayerManager::setPlayerInfo(const std::string& username, Color color) {
    localPlayerInfo.username = username;
    localPlayerInfo.color = color;
    
    if (currentState == MultiplayerState::InLobby) {
        // Send updated player info to server
        NetworkMessage msg(MessageType::PlayerInfo);
        msg.data = serializeJson(localPlayerInfo);
        network->sendMessage(msg);
    }
}

void MultiplayerManager::setPlayerReady(bool ready) {
    localPlayerInfo.isReady = ready;
    
    if (currentState == MultiplayerState::InLobby) {
        NetworkMessage msg(MessageType::PlayerReady);
        msg.data = serializeJson(localPlayerInfo);
        network->sendMessage(msg);
    }
}

void MultiplayerManager::sendPlayerInput(bool rotateLeft, bool rotateRight) {
    if (currentState != MultiplayerState::InGame) {
        return;
    }
    
    PlayerInput input;
    input.playerId = localPlayerInfo.playerId;
    input.rotateLeft = rotateLeft;
    input.rotateRight = rotateRight;
    input.inputSequence = ++inputSequence;
    
    NetworkMessage msg(MessageType::PlayerInput);
    msg.data = serializeJson(input);
    network->sendMessage(msg);
    
    // Store for client prediction
    if (!network->isServer()) {
        pendingInputs.push_back(input);
        // Keep only recent inputs (last 1 second worth)
        if (pendingInputs.size() > 60) {
            pendingInputs.erase(pendingInputs.begin());
        }
    }
}

void MultiplayerManager::update(float deltaTime) {
    network->update();
    
    if (network->isServer()) {
        updateServerLobby(deltaTime);
        updateServerGame(deltaTime);
    } else {
        updateClientGame(deltaTime);
    }
}

ConnectionStatus MultiplayerManager::getConnectionStatus() const {
    return network->getStatus();
}

bool MultiplayerManager::isServer() const {
    return network->isServer();
}

void MultiplayerManager::onNetworkMessage(const NetworkMessage& message) {
    switch (message.type) {
        case MessageType::PlayerJoin:
            handlePlayerJoin(message);
            break;
        case MessageType::PlayerLeave:
            handlePlayerLeave(message);
            break;
        case MessageType::PlayerInfo:
            handlePlayerInfo(message);
            break;
        case MessageType::PlayerReady:
            handlePlayerReady(message);
            break;
        case MessageType::GameStart:
            handleGameStart(message);
            break;
        case MessageType::LobbyState:
            handleLobbyState(message);
            break;
        case MessageType::PlayerInput:
            handlePlayerInput(message);
            break;
        case MessageType::GameState:
            handleGameState(message);
            break;
        case MessageType::PaintStroke:
            handlePaintStroke(message);
            break;
        default:
            break;
    }
}

void MultiplayerManager::onConnectionStatusChanged(ConnectionStatus status) {
    if (status == ConnectionStatus::Connected && currentState == MultiplayerState::Connecting) {
        setState(MultiplayerState::InLobby);
        
        // Send initial player info
        localPlayerInfo.playerId = network->getLocalPlayerId();
        NetworkMessage msg(MessageType::PlayerInfo);
        msg.data = serializeJson(localPlayerInfo);
        network->sendMessage(msg);
    } else if (status == ConnectionStatus::Disconnected) {
        setState(MultiplayerState::Disconnected);
    }
}

void MultiplayerManager::handlePlayerJoin(const NetworkMessage& message) {
    if (!network->isServer()) return;
    
    LOG_INFO_MSG("Player %d joined", message.playerId);
    // Player join is handled in the network layer, just broadcast lobby state
    broadcastLobbyState();
}

void MultiplayerManager::handlePlayerLeave(const NetworkMessage& message) {
    // Remove player from lobby
    auto it = std::find_if(lobbyState.players.begin(), lobbyState.players.end(),
        [&](const PlayerInfo& p) { return p.playerId == message.playerId; });
    
    if (it != lobbyState.players.end()) {
        lobbyState.players.erase(it);
        
        if (lobbyUpdateHandler) {
            lobbyUpdateHandler(lobbyState);
        }
    }
    
    // Remove player entity if in game
    auto entityIt = playerEntities.find(message.playerId);
    if (entityIt != playerEntities.end()) {
        if (registry.valid(entityIt->second)) {
            registry.destroy(entityIt->second);
        }
        playerEntities.erase(entityIt);
    }
    
    LOG_INFO_MSG("Player %d left", message.playerId);
}

void MultiplayerManager::handlePlayerInfo(const NetworkMessage& message) {
    if (!network->isServer()) return;
    
    PlayerInfo playerInfo = deserializeJson<PlayerInfo>(message.data);
    playerInfo.playerId = message.playerId; // Ensure correct player ID
    
    // Update or add player info
    auto it = std::find_if(lobbyState.players.begin(), lobbyState.players.end(),
        [&](const PlayerInfo& p) { return p.playerId == playerInfo.playerId; });
    
    if (it != lobbyState.players.end()) {
        *it = playerInfo;
    } else {
        lobbyState.players.push_back(playerInfo);
    }
    
    broadcastLobbyState();
}

void MultiplayerManager::handlePlayerReady(const NetworkMessage& message) {
    if (!network->isServer()) return;
    
    PlayerInfo playerInfo = deserializeJson<PlayerInfo>(message.data);
    
    // Update player ready status
    auto it = std::find_if(lobbyState.players.begin(), lobbyState.players.end(),
        [&](const PlayerInfo& p) { return p.playerId == message.playerId; });
    
    if (it != lobbyState.players.end()) {
        it->isReady = playerInfo.isReady;
        broadcastLobbyState();
        checkGameStart();
    }
}

void MultiplayerManager::handleGameStart(const NetworkMessage& message) {
    if (network->isServer()) return;
    
    LOG_DEBUG_MSG("Client: Received GameStart message");
    GameStartInfo startInfo = deserializeJson<GameStartInfo>(message.data);
    LOG_INFO_MSG("Client: Starting game with %zu players", startInfo.players.size());
    
    // Transition to networked game scene
    SceneTransitionSystem::requestTransition(registry, SceneType::NetworkedGame);
    setState(MultiplayerState::InGame);
    
    // Create player entities
    auto spawnPositions = getSpawnPositions();
    for (size_t i = 0; i < startInfo.players.size() && i < spawnPositions.size(); ++i) {
        LOG_DEBUG_MSG("Client: Creating player entity for player %d at position (%.2f, %.2f)", 
                      startInfo.players[i].playerId, spawnPositions[i].x, spawnPositions[i].y);
        createPlayerEntity(startInfo.players[i], spawnPositions[i]);
    }
    
    if (gameStartHandler) {
        gameStartHandler(startInfo);
    }
}

void MultiplayerManager::handleLobbyState(const NetworkMessage& message) {
    if (network->isServer()) return;
    
    lobbyState = deserializeJson<LobbyState>(message.data);
    
    if (lobbyUpdateHandler) {
        lobbyUpdateHandler(lobbyState);
    }
}

void MultiplayerManager::handlePlayerInput(const NetworkMessage& message) {
    if (!network->isServer()) return;
    
    PlayerInput input = deserializeJson<PlayerInput>(message.data);
    LOG_DEBUG_MSG("Server: Received input from player %d - Left=%d, Right=%d", 
                  input.playerId, input.rotateLeft, input.rotateRight);
    
    // Apply input to player entity
    auto entityIt = playerEntities.find(input.playerId);
    if (entityIt != playerEntities.end() && registry.valid(entityIt->second)) {
        LOG_DEBUG_MSG("Server: Applying input to entity %u", static_cast<uint32_t>(entityIt->second));
        auto& inputAction = registry.get<InputAction>(entityIt->second);
        inputAction.rotateLeft = input.rotateLeft;
        inputAction.rotateRight = input.rotateRight;
    } else {
        LOG_WARN_MSG("Server: Could not find entity for player %d", input.playerId);
    }
}

void MultiplayerManager::handleGameState(const NetworkMessage& message) {
    if (network->isServer()) return;
    
    GameState gameState = deserializeJson<GameState>(message.data);
    LOG_DEBUG_MSG("Client: Received game state with %zu players", gameState.players.size());
    
    // Only process if this is a newer state
    if (gameState.stateSequence > lastStateSequence) {
        lastStateSequence = gameState.stateSequence;
        LOG_DEBUG_MSG("Client: Applying server state (sequence %u)", gameState.stateSequence);
        reconcileWithServerState(gameState);
    } else {
        LOG_DEBUG_MSG("Client: Ignoring old game state (sequence %u <= %u)", gameState.stateSequence, lastStateSequence);
    }
}

void MultiplayerManager::updateServerLobby(float deltaTime) {
    if (currentState != MultiplayerState::InLobby) return;
    
    if (countdownActive) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - countdownStartTime).count();
        
        if (elapsed >= 3) { // 3 second countdown
            // Start the game
            LOG_INFO_MSG("Server: Starting game with %zu players", lobbyState.players.size());
            GameStartInfo startInfo;
            startInfo.players = lobbyState.players;
            startInfo.gameId = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
            
            NetworkMessage msg(MessageType::GameStart);
            msg.data = serializeJson(startInfo);
            network->broadcastMessage(msg);
            
            // Transition server to networked game
            SceneTransitionSystem::requestTransition(registry, SceneType::NetworkedGame);
            setState(MultiplayerState::InGame);
            
            // Create player entities on server
            auto spawnPositions = getSpawnPositions();
            for (size_t i = 0; i < startInfo.players.size() && i < spawnPositions.size(); ++i) {
                LOG_DEBUG_MSG("Server: Creating player entity for player %u at position (%.2f, %.2f)", 
                              startInfo.players[i].playerId, spawnPositions[i].x, spawnPositions[i].y);
                createPlayerEntity(startInfo.players[i], spawnPositions[i]);
            }
            
            countdownActive = false;
            gameStartTime = std::chrono::steady_clock::now();
            
            if (gameStartHandler) {
                gameStartHandler(startInfo);
            }
        } else {
            // Update countdown
            lobbyState.countdown = 3 - static_cast<uint32_t>(elapsed);
            broadcastLobbyState();
        }
    }
}

void MultiplayerManager::updateServerGame(float deltaTime) {
    if (currentState != MultiplayerState::InGame) return;
    
    LOG_DEBUG_MSG("Server: Updating game state...");
    
    // Broadcast game state to clients (e.g., 20 times per second)
    static float stateTimer = 0.0f;
    stateTimer += deltaTime;
    
    if (stateTimer >= 0.05f) { // 20 Hz
        LOG_DEBUG_MSG("Server: Broadcasting game state to %zu players", playerEntities.size());
        broadcastGameState();
        stateTimer = 0.0f;
    }
}

void MultiplayerManager::updateClientGame(float deltaTime) {
    if (currentState != MultiplayerState::InGame) return;
    
    // Apply client prediction
    applyClientPrediction();
}

void MultiplayerManager::checkGameStart() {
    if (lobbyState.players.size() < 2) return; // Need at least 2 players
    
    // Check if all players are ready
    bool allReady = true;
    for (const auto& player : lobbyState.players) {
        if (!player.isReady) {
            allReady = false;
            break;
        }
    }
    
    if (allReady && !countdownActive) {
        startCountdown();
    }
}

void MultiplayerManager::startCountdown() {
    countdownActive = true;
    countdownStartTime = std::chrono::steady_clock::now();
    lobbyState.gameStarting = true;
    lobbyState.countdown = 3;
    broadcastLobbyState();
}

void MultiplayerManager::broadcastLobbyState() {
    NetworkMessage msg(MessageType::LobbyState);
    msg.data = serializeJson(lobbyState);
    network->broadcastMessage(msg);
}

void MultiplayerManager::broadcastGameState() {
    GameState gameState;
    gameState.stateSequence = ++lastStateSequence;
    
    auto now = std::chrono::steady_clock::now();
    gameState.gameTime = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - gameStartTime).count());
    
    LOG_DEBUG_MSG("Server: Broadcasting game state (sequence %u)", gameState.stateSequence);
    LOG_DEBUG_MSG("Server: Found %zu player entities", playerEntities.size());
    
    // Collect player states
    for (const auto& [playerId, entity] : playerEntities) {
        if (registry.valid(entity)) {
            PlayerState playerState;
            playerState.playerId = playerId;
            
            if (auto* pos = registry.try_get<Position>(entity)) {
                playerState.position = pos->position;
                LOG_DEBUG_MSG("Server: Player %u at position (%.2f, %.2f)", playerId, pos->position.x, pos->position.y);
            }
            
            if (auto* vel = registry.try_get<Velocity>(entity)) {
                playerState.velocity = vel->velocity;
                playerState.rotation = vel->rotation;
            }
            
            if (auto* col = registry.try_get<CollisionState>(entity)) {
                playerState.isInCollision = col->isInCollision;
            }
            
            gameState.players.push_back(playerState);
        } else {
            LOG_WARN_MSG("Server: Player %u has invalid entity!", playerId);
        }
    }
    
    LOG_DEBUG_MSG("Server: Sending game state with %zu players", gameState.players.size());
    
    NetworkMessage msg(MessageType::GameState);
    msg.data = serializeJson(gameState);
    network->broadcastMessage(msg);
}

void MultiplayerManager::createPlayerEntity(const PlayerInfo& playerInfo, const Vector2& spawnPos) {
    LOG_DEBUG_MSG("Creating player entity: ID=%u, Username=%s, Position=(%.2f, %.2f)", 
                  playerInfo.playerId, playerInfo.username.c_str(), spawnPos.x, spawnPos.y);
              
    const auto player = registry.create();
    
    registry.emplace<Position>(player, Position{.position = spawnPos});
    registry.emplace<Velocity>(player, Velocity{
        .velocity = {0, 0},
        .rotation = 0.0f,
        .speed = gameConfig.brushMovementSpeed,
        .rotationSpeed = 120.0F
    });
    registry.emplace<Renderable>(player, Renderable{
        .radius = gameConfig.brushSize,
        .color = playerInfo.color
    });
    registry.emplace<InputAction>(player, InputAction{
        .rotateLeft = false,
        .rotateRight = false
    });
    registry.emplace<CollisionState>(player, CollisionState{
        .isInCollision = false,
        .bounceTimer = 0.0F,
        .bounceVelocity = Vector2{0, 0}
    });
    registry.emplace<Username>(player, Username{
        .text = playerInfo.username
    });
    
    // Add input mapping for local player (both client and server)
    if (playerInfo.playerId == localPlayerInfo.playerId) {
        LOG_DEBUG_MSG("Adding input mapping for local player %u (%s)", 
                      playerInfo.playerId, (network->isServer() ? "server" : "client"));
        registry.emplace<InputMapping>(player, InputMapping{
            .rotateLeftKey = KEY_A,
            .rotateRightKey = KEY_D
        });
    }
    
    EntityLifecycleSystem::tagEntityWithScene(registry, player, SceneType::NetworkedGame);
    playerEntities[playerInfo.playerId] = player;
    
    LOG_DEBUG_MSG("Player entity created successfully with entity ID: %u", static_cast<uint32_t>(player));
}

void MultiplayerManager::applyClientPrediction() {
    // Find local player entity
    if (const auto entityIt = playerEntities.find(localPlayerInfo.playerId);
        entityIt == playerEntities.end() || !registry.valid(entityIt->second)) {
        return;
    }
    
    // Apply pending inputs for prediction
    // This is a simplified version
    // TODO:
    // to store game state snapshots and replay inputs from the server's
    // acknowledged state
}

void MultiplayerManager::reconcileWithServerState(const GameState& serverState) {
    LOG_DEBUG_MSG("Client: Reconciling with server state for %zu players", serverState.players.size());
    LOG_DEBUG_MSG("Client: Local player ID is %u", localPlayerInfo.playerId);
    
    // Update player entities with server state
    for (const auto& playerState : serverState.players) {
        auto entityIt = playerEntities.find(playerState.playerId);
        if (entityIt != playerEntities.end() && registry.valid(entityIt->second)) {
            LOG_DEBUG_MSG("Client: Updating entity %u for player %u to position (%.2f, %.2f) %s", 
                          static_cast<uint32_t>(entityIt->second), playerState.playerId, 
                          playerState.position.x, playerState.position.y,
                          (playerState.playerId == localPlayerInfo.playerId ? "(LOCAL)" : "(REMOTE)"));
                      
            // Update position and velocity from server
            if (auto* pos = registry.try_get<Position>(entityIt->second)) {
                pos->position = playerState.position;
            }
            
            if (auto* vel = registry.try_get<Velocity>(entityIt->second)) {
                vel->velocity = playerState.velocity;
                vel->rotation = playerState.rotation;
            }
            
            if (auto* col = registry.try_get<CollisionState>(entityIt->second)) {
                col->isInCollision = playerState.isInCollision;
            }
        } else {
            LOG_WARN_MSG("Client: Could not find entity for player %u", playerState.playerId);
        }
    }
}

void MultiplayerManager::setState(MultiplayerState newState) {
    if (currentState != newState) {
        currentState = newState;
        if (stateChangeHandler) {
            stateChangeHandler(currentState);
        }
    }
}

PlayerInfo* MultiplayerManager::findPlayerInfo(uint32_t playerId) {
    auto it = std::find_if(lobbyState.players.begin(), lobbyState.players.end(),
        [playerId](const PlayerInfo& p) { return p.playerId == playerId; });
    return it != lobbyState.players.end() ? &(*it) : nullptr;
}

Vector2 MultiplayerManager::getSpawnPosition(size_t playerIndex) {
    // Same spawn positions as local game
    std::vector<Vector2> positions = {
        {100, 100},   // Top-left
        {1180, 100},  // Top-right
        {1180, 620},  // Bottom-right
        {100, 620}    // Bottom-left
    };
    
    if (playerIndex < positions.size()) {
        return positions[playerIndex];
    }
    
    // Fallback for additional players
    return {400 + static_cast<float>(playerIndex * 100), 300};
}

std::vector<Vector2> MultiplayerManager::getSpawnPositions() {
    return {
        {100, 100},   // Top-left
        {1180, 100},  // Top-right
        {1180, 620},  // Bottom-right
        {100, 620}    // Bottom-left
    };
}

void MultiplayerManager::sendPaintStroke(uint32_t playerId, Vector2 position, float radius, Color color) {
    if (network->getStatus() != ConnectionStatus::Connected) return;

    const PaintStroke stroke {
        .playerId = playerId,
        .position = position,
        .radius = radius,
        .color = color,
    };
    
    NetworkMessage message(MessageType::PaintStroke, playerId);
    message.data = serializeJson(stroke);
    
    LOG_DEBUG_MSG("Sending paint stroke from player %u at position (%.2f, %.2f) with radius %.2f", 
                  playerId, position.x, position.y, radius);
    
    if (network->isServer()) {
        network->broadcastMessage(message);
    } else {
        network->sendMessage(message);
    }
}

void MultiplayerManager::handlePaintStroke(const NetworkMessage& message) {
    const auto stroke = deserializeJson<PaintStroke>(message.data);
    
    LOG_DEBUG_MSG("Received paint stroke from player %u at position (%.2f, %.2f) with radius %.2f", 
                  stroke.playerId, stroke.position.x, stroke.position.y, stroke.radius);
    
    // If we're the server, broadcast the stroke to all other clients
    if (network->isServer()) {
        NetworkMessage forwardMessage(MessageType::PaintStroke, stroke.playerId);
        forwardMessage.data = message.data;  // Forward the original data
        network->broadcastMessage(forwardMessage);
    }
    
    // Apply the paint stroke to our local render texture
    // TODO: notify the paint system about this stroke
    // For now, storing it and let the paint system poll for strokes
    receivedPaintStrokes.push_back(stroke);
}

std::vector<PaintStroke> MultiplayerManager::getAndClearReceivedPaintStrokes() {
    std::vector<PaintStroke> strokes = std::move(receivedPaintStrokes);
    receivedPaintStrokes.clear();
    return strokes;
}

bool MultiplayerManager::isLocalPlayerEntity(entt::entity entity) const {
    auto entityIt = playerEntities.find(localPlayerInfo.playerId);
    return entityIt != playerEntities.end() && entityIt->second == entity;
}

} // namespace network