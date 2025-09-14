#pragma once

#include "socket.hpp"
#include <memory>
#include <functional>
#include <string>

namespace scuffedredis {
namespace network {

// TCP server for handling multiple client connections
// Used when we need to accept and manage client connections efficiently
class TcpServer {
public:
    using ClientHandler = std::function<void(std::unique_ptr<Socket>)>;
    
    TcpServer(const std::string& host = "0.0.0.0", int port = 6379);
    ~TcpServer();
    
    // Start the server
    bool start();
    
    // Stop the server
    void stop();
    
    // Set client connection handler
    void setClientHandler(ClientHandler handler) { clientHandler_ = handler; }
    
    // Check if server is running
    bool isRunning() const { return running_; }
    
    // Get server info
    std::string getHost() const { return host_; }
    int getPort() const { return port_; }

private:
    std::string host_;
    int port_;
    std::unique_ptr<Socket> serverSocket_;
    bool running_;
    ClientHandler clientHandler_;
    
    // Accept incoming connections
    void acceptConnections();
};

} // namespace network
} // namespace scuffedredis
