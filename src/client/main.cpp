/**
 * ScuffedRedis CLI Client
 * 
 * Command-line interface for interacting with the Redis server.
 * Supports interactive mode and single command execution.
 */

#include "network/tcp_client.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <sstream>

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
 * Handles user input and displays server responses.
 */
void run_interactive_mode(TcpClient& client) {
    std::cout << "Type 'help' for commands, 'quit' to exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::string line;
    int command_num = 1;
    
    while (client.is_connected()) {
        // Display prompt
        std::cout << client.get_server_info() << ":" << command_num << "> ";
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
        
        // Handle special commands
        if (line == "quit" || line == "exit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        
        if (line == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  help    - Show this help message" << std::endl;
            std::cout << "  quit    - Exit the client" << std::endl;
            std::cout << "  exit    - Exit the client" << std::endl;
            std::cout << "  ping    - Test server connection" << std::endl;
            std::cout << "  echo <message> - Echo a message (test command)" << std::endl;
            std::cout << "\nRedis commands will be implemented soon:" << std::endl;
            std::cout << "  SET key value" << std::endl;
            std::cout << "  GET key" << std::endl;
            std::cout << "  DEL key" << std::endl;
            std::cout << "  EXISTS key" << std::endl;
            std::cout << "  KEYS pattern" << std::endl;
            command_num++;
            continue;
        }
        
        // Send command to server
        std::string response = client.send_and_receive(line, 5000);
        
        if (response.empty()) {
            std::cout << "(error) No response from server or connection lost" << std::endl;
            break;
        }
        
        // Display response
        std::cout << response;
        if (!response.empty() && response.back() != '\n') {
            std::cout << std::endl;
        }
        
        command_num++;
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
    
    std::cout << "ScuffedRedis CLI v0.1.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Create client
    TcpClient client;
    
    // Attempt to connect
    std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
    
    if (!client.connect(host, port, 5000)) {
        std::cerr << "Failed to connect to server at " << host << ":" << port << std::endl;
        std::cerr << "Make sure the server is running and accessible" << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully!" << std::endl;
    
    // Test connection with a ping
    std::string response = client.send_and_receive("ping", 1000);
    if (!response.empty()) {
        std::cout << "Server responded: " << response;
        if (response.back() != '\n') std::cout << std::endl;
    }
    
    // Run interactive mode
    run_interactive_mode(client);
    
    // Cleanup
    client.disconnect();
    
    return 0;
}
