/**
 * ScuffedRedis Server
 * 
 * Entry point for the Redis server implementation.
 * Will evolve from basic socket server to full event-driven architecture.
 */

#include "network/tcp_server.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <atomic>
#include <algorithm>

using namespace scuffedredis;

// Global server instance for signal handling
static TcpServer* g_server = nullptr;
static std::atomic<bool> g_running(false);

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        std::cout << "\nReceived shutdown signal, stopping server..." << std::endl;
        g_running = false;
        if (g_server) {
            g_server->stop();
        }
    }
}

/**
 * Simple echo handler for testing.
 * Echoes back whatever the client sends.
 * Will be replaced with proper Redis protocol handler.
 */
bool echo_handler(ClientConnection& client) {
    const auto& buffer = client.get_read_buffer();
    
    if (buffer.empty()) {
        return true;  // Keep connection open
    }
    
    // For now, just echo back the data
    // Look for newline to determine complete message
    auto it = std::find(buffer.begin(), buffer.end(), '\n');
    if (it != buffer.end()) {
        size_t msg_len = std::distance(buffer.begin(), it) + 1;
        
        // Echo the message back
        std::string response = "ECHO: ";
        client.write(response);
        client.write(buffer.data(), msg_len);
        
        // Consume the processed bytes
        client.consume_bytes(msg_len);
    }
    
    return true;  // Keep connection open
}

int main(int argc, char* argv[]) {
    // Default configuration
    std::string bind_address = "0.0.0.0";  // Listen on all interfaces
    int port = 6380;  // Default port (6379 is standard Redis)
    
    // Parse command line arguments
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    if (argc > 2) {
        bind_address = argv[2];
    }
    
    std::cout << "ScuffedRedis Server v0.1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create and initialize server
    TcpServer server;
    g_server = &server;
    
    if (!server.init(bind_address, port)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    std::cout << "Server listening on " << bind_address << ":" << port << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Run server with echo handler
    g_running = true;
    server.run_blocking(echo_handler);
    
    std::cout << "Server stopped successfully" << std::endl;
    
    g_server = nullptr;
    return 0;
}
