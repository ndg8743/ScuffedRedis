/**
 * ScuffedRedis Server
 * 
 * Entry point for the Redis server implementation.
 * Now includes full Redis command support with KV store.
 */

#include "network/tcp_server.hpp"
#include "server/command_handler.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <atomic>

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
    
    // Configure logging
    Logger::instance().set_level(LogLevel::INFO);
    
    std::cout << "ScuffedRedis Server v0.2.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Full Redis command support enabled" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create and initialize server
    TcpServer server;
    g_server = &server;
    
    if (!server.init(bind_address, port)) {
        LOG_FATAL("Failed to initialize server");
        return 1;
    }
    
    std::cout << "Server listening on " << bind_address << ":" << port << std::endl;
    std::cout << "Supported commands: GET, SET, DEL, EXISTS, KEYS, PING, ECHO, INFO" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Run server with Redis command handler
    g_running = true;
    server.run_blocking(make_command_handler());
    
    std::cout << "Server stopped successfully" << std::endl;
    
    g_server = nullptr;
    return 0;
}
