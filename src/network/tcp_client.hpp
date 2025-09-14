#pragma once

#include "socket.hpp"
#include <string>
#include <memory>

namespace scuffedredis {
namespace network {

// TCP client for connecting to Redis server
// Basically just a wrapper around socket with connection management
class TcpClient {
public:
    TcpClient();
    ~TcpClient();
    
    // Connect to server
    bool connect(const std::string& host, int port);
    
    // Disconnect from server
    void disconnect();
    
    // Send data to server
    ssize_t send(const void* data, size_t len);
    
    // Receive data from server
    ssize_t recv(void* buffer, size_t len);
    
    // Check if connected
    bool isConnected() const { return socket_ && socket_->isValid(); }
    
    // Get connection info
    std::string getHost() const { return host_; }
    int getPort() const { return port_; }

private:
    std::unique_ptr<Socket> socket_;
    std::string host_;
    int port_;
};

} // namespace network
} // namespace scuffedredis
