#include <iostream>
#include <signal.h>
#include <memory>
#include "../network/tcp_server.hpp"
#include "../protocol/request_handler.hpp"
#include "../protocol/request.hpp"
#include "../protocol/response.hpp"
#include "../utils/logger.hpp"

namespace scuffedredis {
namespace server {

// Global server instance for signal handling
std::unique_ptr<network::TcpServer> g_server;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        utils::Logger::info("Received signal {}, shutting down gracefully...", signal);
        if (g_server) {
            g_server->stop();
        }
    }
}

// Handle client connection
void handleClient(std::unique_ptr<network::Socket> clientSocket) {
    if (!clientSocket || !clientSocket->isValid()) {
        utils::Logger::error("Invalid client socket");
        return;
    }
    
    utils::Logger::info("New client connected");
    
    // Create request handler for this client
    protocol::RequestHandler handler;
    
    // Buffer for reading data
    char buffer[4096];
    
    try {
        while (clientSocket->isValid()) {
            // Read request from client
            ssize_t bytesRead = clientSocket->recv(buffer, sizeof(buffer) - 1);
            
            if (bytesRead <= 0) {
                if (bytesRead == 0) {
                    utils::Logger::info("Client disconnected");
                } else {
                    utils::Logger::error("Error reading from client: {}", strerror(errno));
                }
                break;
            }
            
            // Null-terminate the buffer
            buffer[bytesRead] = '\0';
            
            // Parse and handle request
            std::string requestStr(buffer);
            protocol::Request request(requestStr);
            
            if (request.isValid()) {
                // Handle quit command
                if (request.getType() == protocol::Request::QUIT) {
                    protocol::Response response = handler.handleRequest(request);
                    std::string responseStr = response.serialize();
                    clientSocket->send(responseStr.c_str(), responseStr.length());
                    utils::Logger::info("Client requested quit");
                    break;
                }
                
                // Process request and send response
                protocol::Response response = handler.handleRequest(request);
                std::string responseStr = response.serialize();
                
                ssize_t bytesSent = clientSocket->send(responseStr.c_str(), responseStr.length());
                if (bytesSent < 0) {
                    utils::Logger::error("Error sending response to client");
                    break;
                }
                
                utils::Logger::debug("Sent response: {} bytes", bytesSent);
            } else {
                // Send error response for invalid request
                protocol::Response errorResponse = protocol::Response::error("Invalid request format");
                std::string errorStr = errorResponse.serialize();
                clientSocket->send(errorStr.c_str(), errorStr.length());
            }
        }
    } catch (const std::exception& e) {
        utils::Logger::error("Exception in client handler: {}", e.what());
    }
    
    utils::Logger::info("Client handler finished");
}

} // namespace server
} // namespace scuffedredis

int main(int argc, char* argv[]) {
    using namespace scuffedredis;
    
    // Parse command line arguments
    std::string host = "0.0.0.0";
    int port = 6379;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--host HOST] [--port PORT] [--help]\n";
            std::cout << "  --host HOST    Bind to host (default: 0.0.0.0)\n";
            std::cout << "  --port PORT    Bind to port (default: 6379)\n";
            std::cout << "  --help         Show this help message\n";
            return 0;
        }
    }
    
    // Set up logging
    utils::Logger::setLevel(utils::Logger::INFO);
    
    // Set up signal handlers
    signal(SIGINT, server::signalHandler);
    signal(SIGTERM, server::signalHandler);
    
    utils::Logger::info("Starting ScuffedRedis server on {}:{}", host, port);
    
    // Create and start server
    server::g_server = std::make_unique<network::TcpServer>(host, port);
    server::g_server->setClientHandler(server::handleClient);
    
    if (!server::g_server->start()) {
        utils::Logger::error("Failed to start server");
        return 1;
    }
    
    utils::Logger::info("ScuffedRedis server is running. Press Ctrl+C to stop.");
    
    // Keep main thread alive
    while (server::g_server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    utils::Logger::info("ScuffedRedis server stopped");
    return 0;
}
