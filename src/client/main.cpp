#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "../network/tcp_client.hpp"
#include "../utils/logger.hpp"

namespace scuffedredis {
namespace client {

// Simple interactive client
// Basically just a command-line interface to test the server
class RedisClient {
public:
    RedisClient() = default;
    
    bool connect(const std::string& host, int port) {
        return client_.connect(host, port);
    }
    
    void disconnect() {
        client_.disconnect();
    }
    
    std::string sendCommand(const std::string& command) {
        if (!client_.isConnected()) {
            return "Error: Not connected to server";
        }
        
        // Send command
        ssize_t sent = client_.send(command.c_str(), command.length());
        if (sent < 0) {
            return "Error: Failed to send command";
        }
        
        // Receive response
        char buffer[4096];
        ssize_t received = client_.recv(buffer, sizeof(buffer) - 1);
        if (received < 0) {
            return "Error: Failed to receive response";
        }
        
        buffer[received] = '\0';
        return std::string(buffer);
    }
    
    bool isConnected() const {
        return client_.isConnected();
    }

private:
    network::TcpClient client_;
};

} // namespace client
} // namespace scuffedredis

int main(int argc, char* argv[]) {
    using namespace scuffedredis;
    
    // Parse command line arguments
    std::string host = "localhost";
    int port = 6379;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--host HOST] [--port PORT] [--help]\n";
            std::cout << "  --host HOST    Connect to host (default: localhost)\n";
            std::cout << "  --port PORT    Connect to port (default: 6379)\n";
            std::cout << "  --help         Show this help message\n";
            return 0;
        }
    }
    
    // Set up logging
    utils::Logger::setLevel(utils::Logger::WARN);  // Less verbose for client
    
    client::RedisClient client;
    
    std::cout << "ScuffedRedis Client\n";
    std::cout << "Connecting to " << host << ":" << port << "...\n";
    
    if (!client.connect(host, port)) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }
    
    std::cout << "Connected! Type commands or 'quit' to exit.\n";
    std::cout << "Example commands:\n";
    std::cout << "  SET key value\n";
    std::cout << "  GET key\n";
    std::cout << "  DEL key\n";
    std::cout << "  EXISTS key\n";
    std::cout << "  KEYS\n";
    std::cout << "  PING\n";
    std::cout << "  QUIT\n\n";
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input.empty()) {
            continue;
        }
        
        if (input == "quit" || input == "exit") {
            client.sendCommand("QUIT");
            break;
        }
        
        std::string response = client.sendCommand(input);
        std::cout << response << "\n";
        
        if (!client.isConnected()) {
            std::cout << "Connection lost.\n";
            break;
        }
    }
    
    client.disconnect();
    std::cout << "Goodbye!\n";
    return 0;
}
