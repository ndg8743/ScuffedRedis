/**
 * ScuffedRedis Server
 * 
 * Entry point for the Redis server implementation.
 * Will evolve from basic socket server to full event-driven architecture.
 */

#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Default port for Redis is 6379, we'll use 6380 to avoid conflicts
    int port = 6380;
    
    // Parse command line arguments if provided
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    std::cout << "ScuffedRedis Server v0.1.0" << std::endl;
    std::cout << "Starting server on port " << port << "..." << std::endl;
    
    // TODO: Initialize socket server
    // TODO: Set up event loop
    // TODO: Start accepting connections
    
    std::cout << "Server shutting down..." << std::endl;
    return 0;
}
