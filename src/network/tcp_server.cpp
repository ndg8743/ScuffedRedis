#include "tcp_server.hpp"
#include "../utils/logger.hpp"
#include <thread>

namespace scuffedredis {
namespace network {

TcpServer::TcpServer(const std::string& host, int port) 
    : host_(host), port_(port), running_(false) {
    serverSocket_ = std::make_unique<Socket>();
}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start() {
    if (running_) {
        utils::Logger::warn("Server is already running");
        return true;
    }
    
    // Configure server socket for performance
    if (!serverSocket_->setReuseAddr(true)) {
        utils::Logger::error("Failed to set SO_REUSEADDR");
        return false;
    }
    
    if (!serverSocket_->setKeepAlive(true)) {
        utils::Logger::error("Failed to set SO_KEEPALIVE");
        return false;
    }
    
    // Bind and listen
    if (!serverSocket_->bind(host_, port_)) {
        utils::Logger::error("Failed to bind server socket");
        return false;
    }
    
    if (!serverSocket_->listen()) {
        utils::Logger::error("Failed to listen on server socket");
        return false;
    }
    
    running_ = true;
    utils::Logger::info("TCP Server started on {}:{}", host_, port_);
    
    // Start accepting connections in a separate thread
    std::thread acceptThread([this]() {
        acceptConnections();
    });
    acceptThread.detach();
    
    return true;
}

void TcpServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    serverSocket_->close();
    utils::Logger::info("TCP Server stopped");
}

void TcpServer::acceptConnections() {
    utils::Logger::info("Started accepting connections");
    
    while (running_) {
        auto clientSocket = serverSocket_->accept();
        if (clientSocket && clientSocket->isValid()) {
            utils::Logger::debug("New client connected");
            
            // Handle client in separate thread for concurrency
            if (clientHandler_) {
                std::thread clientThread([this, socket = std::move(clientSocket)]() {
                    clientHandler_(std::move(socket));
                });
                clientThread.detach();
            }
        }
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    utils::Logger::info("Stopped accepting connections");
}

} // namespace network
} // namespace scuffedredis
