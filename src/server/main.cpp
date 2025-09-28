// ScuffedRedis Server - Main entry point

#include "network/tcp_server.hpp"
#include "server/command_handler.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <atomic>

using namespace scuffedredis;

// Global server for signal handling
static TcpServer* g_server = nullptr;

// Handle shutdown signals
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        std::cout << "\nShutting down..." << std::endl;
        if (g_server) {
            g_server->stop();
        }
    }
}


int main(int argc, char* argv[]) {
    std::string bind_address = "0.0.0.0";
    int port = 6379;
    
    if (argc > 1) port = std::atoi(argv[1]);
    if (argc > 2) bind_address = argv[2];
    
    Logger::instance().set_level(LogLevel::INFO);
    
    std::cout << "ScuffedRedis Server v1.0.0" << std::endl;
    std::cout << "Server initialized on " << bind_address << ":" << port << std::endl;
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    TcpServer server;
    g_server = &server;
    
    if (!server.init(bind_address, port)) {
        LOG_FATAL("Failed to initialize server");
        return 1;
    }
    
    std::cout << "Server listening on " << bind_address << ":" << port << std::endl;
    std::cout << "Supported commands: GET, SET, DEL, EXISTS, KEYS, PING, ECHO, INFO" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    server.run_blocking(make_command_handler());
    
    std::cout << "Server stopped" << std::endl;
    g_server = nullptr;
    return 0;
}
