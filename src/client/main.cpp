/**
 * ScuffedRedis CLI Client
 * 
 * Command-line interface for interacting with the Redis server.
 * Supports interactive mode and single command execution.
 */

#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Default connection parameters
    std::string host = "127.0.0.1";
    int port = 6380;
    
    // Parse command line arguments
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::atoi(argv[2]);
    }
    
    std::cout << "ScuffedRedis CLI v0.1.0" << std::endl;
    std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
    
    // TODO: Establish TCP connection to server
    // TODO: Implement command parsing
    // TODO: Send requests and handle responses
    // TODO: Interactive REPL loop
    
    std::cout << "Not connected. Server implementation pending." << std::endl;
    
    return 0;
}
