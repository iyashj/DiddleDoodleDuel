#ifndef DIDDLEDOODLEDUEL_ENET_NETWORK_H
#define DIDDLEDOODLEDUEL_ENET_NETWORK_H

#include "network_interface.h"
#include <enet/enet.h>
#include <unordered_map>
#include <mutex>

namespace network {



class ENetNetwork : public INetworkInterface {
public:
    ENetNetwork();
    ~ENetNetwork() override;

    bool startServer(uint16_t port, uint32_t maxClients = 4) override;
    bool connectToServer(const std::string& host, uint16_t port) override;
    void disconnect() override;
    ConnectionStatus getStatus() const override;
    
    void sendMessage(const NetworkMessage& message) override;
    void sendMessageToClient(uint32_t clientId, const NetworkMessage& message) override;
    void broadcastMessage(const NetworkMessage& message) override;
    
    void setMessageHandler(MessageHandler handler) override;
    void setConnectionStatusHandler(ConnectionStatusHandler handler) override;
    
    void update() override;
    
    bool isServer() const override { return isServerMode; }
    uint32_t getLocalPlayerId() const override { return localPlayerId; }
    std::vector<uint32_t> getConnectedClients() const override;

private:
    struct ClientInfo {
        ENetPeer* peer;
        uint32_t playerId;
        bool connected;
    };
    
    ENetHost* host = nullptr;
    ENetPeer* serverPeer = nullptr; // For client mode
    
    bool isServerMode = false;
    ConnectionStatus status = ConnectionStatus::Disconnected;
    uint32_t localPlayerId = 0;
    uint32_t nextPlayerId = 1;
    
    std::unordered_map<uint32_t, ClientInfo> clients; // For server mode
    std::unordered_map<ENetPeer*, uint32_t> peerToPlayerId;
    
    MessageHandler messageHandler;
    ConnectionStatusHandler statusHandler;
    
    mutable std::mutex networkMutex;
    
    // Helper methods
    void handleEvent(const ENetEvent& event);
    void onClientConnected(ENetPeer* peer);
    void onClientDisconnected(ENetPeer* peer);
    void onMessageReceived(ENetPeer* peer, ENetPacket* packet);
    
    ENetPacket* serializeMessage(const NetworkMessage& message);
    NetworkMessage deserializeMessage(ENetPacket* packet);
    
    void setStatus(ConnectionStatus newStatus);
    uint32_t assignPlayerId(ENetPeer* peer);
};

}

#endif // DIDDLEDOODLEDUEL_ENET_NETWORK_H