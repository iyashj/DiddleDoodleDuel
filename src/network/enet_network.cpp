#include "enet_network.h"
#include "logging/logger.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace network {

ENetNetwork::ENetNetwork() {
    if (enet_initialize() != 0) {
        throw std::runtime_error("Failed to initialize ENet");
    }
}

ENetNetwork::~ENetNetwork() {
    ENetNetwork::disconnect();
    enet_deinitialize();
}

bool ENetNetwork::startServer(uint16_t port, uint32_t maxClients) {
    std::lock_guard<std::mutex> lock(networkMutex);
    
    if (status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    
    host = enet_host_create(&address, maxClients, 2, 0, 0);
    if (host == nullptr) {
        return false;
    }
    
    isServerMode = true;
    localPlayerId = 0; // Server has ID 0
    setStatus(ConnectionStatus::Connected);
    
    LOG_INFO_MSG("Server started on port %u with max %zu clients", port, maxClients);
    return true;
}

bool ENetNetwork::connectToServer(const std::string& host, uint16_t port) {
    std::lock_guard<std::mutex> lock(networkMutex);
    
    if (status != ConnectionStatus::Disconnected) {
        return false;
    }
    
    // Create client host
    this->host = enet_host_create(nullptr, 1, 2, 0, 0);
    if (this->host == nullptr) {
        return false;
    }
    
    // Set up server address
    ENetAddress address;
    if (enet_address_set_host(&address, host.c_str()) != 0) {
        enet_host_destroy(this->host);
        this->host = nullptr;
        return false;
    }
    address.port = port;
    
    // Connect to server
    serverPeer = enet_host_connect(this->host, &address, 2, 0);
    if (serverPeer == nullptr) {
        enet_host_destroy(this->host);
        this->host = nullptr;
        return false;
    }
    
    isServerMode = false;
    setStatus(ConnectionStatus::Connecting);
    
    LOG_INFO_MSG("Attempting to connect to %s:%u", host.c_str(), port);
    return true;
}

void ENetNetwork::disconnect() {
    std::lock_guard<std::mutex> lock(networkMutex);
    
    if (status == ConnectionStatus::Disconnected) {
        return;
    }
    
    if (isServerMode) {
        // Disconnect all clients
        for (auto& [playerId, clientInfo] : clients) {
            if (clientInfo.connected) {
                enet_peer_disconnect(clientInfo.peer, 0);
            }
        }
        clients.clear();
        peerToPlayerId.clear();
    } else if (serverPeer != nullptr) {
        // Disconnect from server
        enet_peer_disconnect(serverPeer, 0);
        serverPeer = nullptr;
    }
    
    if (host != nullptr) {
        enet_host_destroy(host);
        host = nullptr;
    }
    
    setStatus(ConnectionStatus::Disconnected);
    LOG_INFO_MSG("Disconnected from network");
}

ConnectionStatus ENetNetwork::getStatus() const {
    std::lock_guard<std::mutex> lock(networkMutex);
    return status;
}

void ENetNetwork::sendMessage(const NetworkMessage& message) {
    if (!isServerMode && serverPeer != nullptr) {
        ENetPacket* packet = serializeMessage(message);
        enet_peer_send(serverPeer, 0, packet);
    }
}

void ENetNetwork::sendMessageToClient(uint32_t clientId, const NetworkMessage& message) {
    if (!isServerMode) return;
    
    std::lock_guard<std::mutex> lock(networkMutex);
    auto it = clients.find(clientId);
    if (it != clients.end() && it->second.connected) {
        ENetPacket* packet = serializeMessage(message);
        enet_peer_send(it->second.peer, 0, packet);
    }
}

void ENetNetwork::broadcastMessage(const NetworkMessage& message) {
    if (!isServerMode) return;
    
    std::lock_guard<std::mutex> lock(networkMutex);
    for (const auto& [playerId, clientInfo] : clients) {
        if (clientInfo.connected) {
            ENetPacket* packet = serializeMessage(message);
            enet_peer_send(clientInfo.peer, 0, packet);
        }
    }
}

void ENetNetwork::setMessageHandler(MessageHandler handler) {
    messageHandler = std::move(handler);
}

void ENetNetwork::setConnectionStatusHandler(ConnectionStatusHandler handler) {
    statusHandler = std::move(handler);
}

void ENetNetwork::update() {
    if (host == nullptr) {
        return;
    }
    
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        handleEvent(event);
    }
}

std::vector<uint32_t> ENetNetwork::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(networkMutex);
    std::vector<uint32_t> result;
    
    for (const auto& [playerId, clientInfo] : clients) {
        if (clientInfo.connected) {
            result.push_back(playerId);
        }
    }
    
    return result;
}

void ENetNetwork::handleEvent(const ENetEvent& event) {
    switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            if (isServerMode) {
                onClientConnected(event.peer);
            } else {
                // Client connected to server
                localPlayerId = assignPlayerId(event.peer);
                setStatus(ConnectionStatus::Connected);
                LOG_INFO_MSG("Connected to server");
            }
            break;
            
        case ENET_EVENT_TYPE_DISCONNECT:
            if (isServerMode) {
                onClientDisconnected(event.peer);
            } else {
                // Disconnected from server
                setStatus(ConnectionStatus::Disconnected);
                LOG_INFO_MSG("Disconnected from server");
            }
            break;
            
        case ENET_EVENT_TYPE_RECEIVE:
            onMessageReceived(event.peer, event.packet);
            enet_packet_destroy(event.packet);
            break;
            
        default:
            break;
    }
}

void ENetNetwork::onClientConnected(ENetPeer* peer) {
    uint32_t playerId = assignPlayerId(peer);
    
    ClientInfo clientInfo;
    clientInfo.peer = peer;
    clientInfo.playerId = playerId;
    clientInfo.connected = true;
    
    {
        std::lock_guard<std::mutex> lock(networkMutex);
        clients[playerId] = clientInfo;
        peerToPlayerId[peer] = playerId;
    }
    
    LOG_INFO_MSG("Client connected, assigned player ID: %u", playerId);
    
    // Send welcome message or lobby state
    NetworkMessage welcomeMsg(MessageType::PlayerJoin, playerId);
    sendMessageToClient(playerId, welcomeMsg);
}

void ENetNetwork::onClientDisconnected(ENetPeer* peer) {
    std::lock_guard<std::mutex> lock(networkMutex);
    
    auto peerIt = peerToPlayerId.find(peer);
    if (peerIt != peerToPlayerId.end()) {
        uint32_t playerId = peerIt->second;
        
        clients.erase(playerId);
        peerToPlayerId.erase(peer);
        
        LOG_INFO_MSG("Client disconnected, player ID: %u", playerId);
        
        // Notify other clients
        NetworkMessage leaveMsg(MessageType::PlayerLeave, playerId);
        broadcastMessage(leaveMsg);
    }
}

void ENetNetwork::onMessageReceived(ENetPeer* peer, ENetPacket* packet) {
    if (messageHandler) {
        NetworkMessage message = deserializeMessage(packet);
        
        // Set player ID from peer mapping if not set
        if (message.playerId == 0) {
            std::lock_guard<std::mutex> lock(networkMutex);
            auto it = peerToPlayerId.find(peer);
            if (it != peerToPlayerId.end()) {
                message.playerId = it->second;
            }
        }
        
        messageHandler(message);
    }
}

ENetPacket* ENetNetwork::serializeMessage(const NetworkMessage& message) {
    // Simple serialization: [type][playerId][dataSize][data]
    size_t totalSize = sizeof(MessageType) + sizeof(uint32_t) + sizeof(uint32_t) + message.data.size();
    
    ENetPacket* packet = enet_packet_create(nullptr, totalSize, ENET_PACKET_FLAG_RELIABLE);
    
    uint8_t* buffer = packet->data;
    size_t offset = 0;
    
    // Copy message type
    std::memcpy(buffer + offset, &message.type, sizeof(MessageType));
    offset += sizeof(MessageType);
    
    // Copy player ID
    std::memcpy(buffer + offset, &message.playerId, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // Copy data size
    uint32_t dataSize = static_cast<uint32_t>(message.data.size());
    std::memcpy(buffer + offset, &dataSize, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // Copy data
    if (!message.data.empty()) {
        std::memcpy(buffer + offset, message.data.data(), message.data.size());
    }
    
    return packet;
}

NetworkMessage ENetNetwork::deserializeMessage(ENetPacket* packet) {
    const uint8_t* buffer = packet->data;
    size_t offset = 0;
    
    // Read message type
    MessageType type;
    std::memcpy(&type, buffer + offset, sizeof(MessageType));
    offset += sizeof(MessageType);
    
    // Read player ID
    uint32_t playerId;
    std::memcpy(&playerId, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // Read data size
    uint32_t dataSize;
    std::memcpy(&dataSize, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    NetworkMessage message(type, playerId);
    
    // Read data
    if (dataSize > 0 && offset + dataSize <= packet->dataLength) {
        message.data.resize(dataSize);
        std::memcpy(message.data.data(), buffer + offset, dataSize);
    }
    
    return message;
}

void ENetNetwork::setStatus(ConnectionStatus newStatus) {
    if (status != newStatus) {
        status = newStatus;
        if (statusHandler) {
            statusHandler(status);
        }
    }
}

uint32_t ENetNetwork::assignPlayerId(ENetPeer* peer) {
    return nextPlayerId++;
}

std::unique_ptr<INetworkInterface> createNetworkInterface() {
    return std::make_unique<ENetNetwork>();
}

} // namespace network