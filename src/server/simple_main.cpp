#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "network/socket.hpp"
#include "utils/logger.hpp"

int main() {
    // Initialize socket subsystem
    if (!scuffedredis::initialize_sockets()) {
        std::cerr << "Failed to initialize sockets" << std::endl;
        return 1;
    }

    // Configure logging
    scuffedredis::Logger::instance().set_level(scuffedredis::LogLevel::INFO);
    scuffedredis::Logger::instance().set_show_timestamp(true);

    LOG_INFO("Starting ScuffedRedis Simple Server");

    // Create server socket
    scuffedredis::Socket server_socket;
    if (!server_socket.create_tcp()) {
        LOG_ERROR("Failed to create server socket");
        return 1;
    }

    // Set socket options
    server_socket.set_reuseaddr(true);
    server_socket.set_nodelay(true);

    // Bind to port 6379
    if (!server_socket.bind("0.0.0.0", 6379)) {
        LOG_ERROR("Failed to bind to port 6379");
        return 1;
    }

    // Start listening
    if (!server_socket.listen(10)) {
        LOG_ERROR("Failed to start listening");
        return 1;
    }

    LOG_INFO("ScuffedRedis Simple Server listening on port 6379");

    // Simple accept loop
    while (true) {
        auto client_socket = server_socket.accept();
        if (!client_socket.is_valid()) {
            LOG_ERROR("Failed to accept connection");
            continue;
        }

        LOG_INFO("New client connected");

        // Simple echo server
        char buffer[1024];
        while (true) {
            auto bytes_received = client_socket.recv(buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0) {
                break;
            }

            buffer[bytes_received] = '\0';
            LOG_INFO("Received: " + std::string(buffer));

            // Echo back
            client_socket.send(buffer, bytes_received);
        }

        LOG_INFO("Client disconnected");
    }

    // Cleanup
    scuffedredis::cleanup_sockets();
    return 0;
}
