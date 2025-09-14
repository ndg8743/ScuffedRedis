#include "tcp_client.hpp"
#include "../utils/logger.hpp"

namespace scuffedredis {
namespace network {

TcpClient::TcpClient() : port_(0) {
    socket_ = std::make_unique<Socket>();
}

TcpClient::~TcpClient() {
    disconnect();
}

bool TcpClient::connect(const std::string& host, int port) {
    if (isConnected()) {
        utils::Logger::warn("Client is already connected");
        return true;
    }
    
    host_ = host;
    port_ = port;
    
    if (!socket_->connect(host, port)) {
        utils::Logger::error("Failed to connect to {}:{}", host, port);
        return false;
    }
    
    // Configure socket for performance
    socket_->setKeepAlive(true);
    
    utils::Logger::info("Client connected to {}:{}", host, port);
    return true;
}

void TcpClient::disconnect() {
    if (socket_) {
        socket_->close();
        utils::Logger::info("Client disconnected from {}:{}", host_, port_);
    }
}

ssize_t TcpClient::send(const void* data, size_t len) {
    if (!isConnected()) {
        utils::Logger::error("Cannot send data: not connected");
        return -1;
    }
    
    return socket_->send(data, len);
}

ssize_t TcpClient::recv(void* buffer, size_t len) {
    if (!isConnected()) {
        utils::Logger::error("Cannot receive data: not connected");
        return -1;
    }
    
    return socket_->recv(buffer, len);
}

} // namespace network
} // namespace scuffedredis
