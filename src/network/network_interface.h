#ifndef DIDDLEDOODLEDUEL_NETWORK_INTERFACE_H
#define DIDDLEDOODLEDUEL_NETWORK_INTERFACE_H

#include "message_definitions.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace network {

enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Failed
};

using MessageHandler = std::function<void(const NetworkMessage&)>;
using ConnectionStatusHandler = std::function<void(ConnectionStatus)>;

class INetworkInterface {
public:
    virtual ~INetworkInterface() = default;
    
    // Connection management
    virtual bool startServer(uint16_t port, uint32_t maxClients = 4) = 0;
    virtual bool connectToServer(const std::string& host, uint16_t port) = 0;
    virtual void disconnect() = 0;
    virtual ConnectionStatus getStatus() const = 0;
    
    // Message handling
    virtual void sendMessage(const NetworkMessage& message) = 0;
    virtual void sendMessageToClient(uint32_t clientId, const NetworkMessage& message) = 0;
    virtual void broadcastMessage(const NetworkMessage& message) = 0;
    
    // Event callbacks
    virtual void setMessageHandler(MessageHandler handler) = 0;
    virtual void setConnectionStatusHandler(ConnectionStatusHandler handler) = 0;
    
    // Update loop - should be called regularly
    virtual void update() = 0;
    
    // Utility
    virtual bool isServer() const = 0;
    virtual uint32_t getLocalPlayerId() const = 0;
    virtual std::vector<uint32_t> getConnectedClients() const = 0;
};

// Factory function to create network interface
std::unique_ptr<INetworkInterface> createNetworkInterface();

} // namespace network

#endif // DIDDLEDOODLEDUEL_NETWORK_INTERFACE_H