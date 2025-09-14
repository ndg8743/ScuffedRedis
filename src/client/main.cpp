/**
 * ScuffedRedis CLI Client
 * 
 * Command-line interface for interacting with the Redis server.
 * Uses Redis protocol for communication.
 */

#include "client/redis_client.hpp"
#include "utils/logger.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <vector>

using namespace scuffedredis;

/**
 * Trim whitespace from string.
 * Used to clean up user input.
 */
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

/**
 * Interactive REPL (Read-Eval-Print Loop) for Redis commands.
 * Handles user input and displays server responses using Redis protocol.
 */
void run_interactive_mode(RedisClient& client) {
    std::cout << "Type 'help' for commands, 'quit' to exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::string line;
    int command_num = 1;
    
    while (client.is_connected()) {
        // Display prompt (Redis-style)
        std::cout << "127.0.0.1:6380> ";
        std::cout.flush();
        
        // Read user input
        if (!std::getline(std::cin, line)) {
            // EOF or error
            break;
        }
        
        // Trim whitespace
        line = trim(line);
        
        // Handle empty input
        if (line.empty()) {
            continue;
        }
        
        // Handle special client-side commands
        if (line == "quit" || line == "exit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        if (line == "help") {
            std::cout << "\nRedis Commands:" << std::endl;
            std::cout << "  SET key value      - Set a key to a value" << std::endl;
            std::cout << "  GET key            - Get value of a key" << std::endl;
            std::cout << "  DEL key [key ...]  - Delete one or more keys" << std::endl;
            std::cout << "  EXISTS key [...]   - Check if keys exist" << std::endl;
            std::cout << "  KEYS pattern       - Find keys matching pattern" << std::endl;
            std::cout << "  PING [message]     - Test connection" << std::endl;
            std::cout << "  ECHO message       - Echo a message" << std::endl;
            std::cout << "  FLUSHDB            - Clear all keys" << std::endl;
            std::cout << "  DBSIZE             - Get number of keys" << std::endl;
            std::cout << "  INFO               - Get server information" << std::endl;
            std::cout << "\nClient Commands:" << std::endl;
            std::cout << "  help               - Show this help" << std::endl;
            std::cout << "  quit/exit          - Exit the client" << std::endl;
            std::cout << std::endl;
            continue;
        }
        
        // Parse command line into arguments
        auto args = RedisClient::parse_command_line(line);
        
        if (args.empty()) {
            continue;
        }
        
        // Execute command
        auto response = client.execute(args);
        
        if (!response) {
            std::cout << "(error) Failed to execute command" << std::endl;
            continue;
        }
        
        // Format and display response
        std::cout << RedisClient::format_response(response) << std::endl;
    }
    
    if (!client.is_connected()) {
        std::cout << "\nConnection to server lost" << std::endl;
    }
}

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
    
    // Configure logging (only show errors for client)
    Logger::instance().set_level(LogLevel::ERROR);
    
    std::cout << "ScuffedRedis CLI v0.2.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Create Redis client
    RedisClient client;
    
    // Attempt to connect
    std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
    
    if (!client.connect(host, static_cast<uint16_t>(port), 5000)) {
        std::cerr << "Failed to connect to server at " << host << ":" << port << std::endl;
        std::cerr << "Make sure the server is running and accessible" << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully!" << std::endl;
    
    // Test connection with a ping
    if (client.ping()) {
        std::cout << "Server is ready" << std::endl;
    } else {
        std::cerr << "Warning: Server did not respond to PING" << std::endl;
    }
    
    // Run interactive mode
    run_interactive_mode(client);
    
    // Cleanup
    client.disconnect();
    
    return 0;
}
