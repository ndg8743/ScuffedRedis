#ifndef SCUFFEDREDIS_REDIS_CLIENT_HPP
#define SCUFFEDREDIS_REDIS_CLIENT_HPP

/**
 * Redis client with protocol support.
 * 
 * Handles command serialization and response parsing.
 * Provides high-level interface for Redis operations.
 */

#include "network/tcp_client.hpp"
#include "protocol/protocol.hpp"
#include <string>
#include <vector>
#include <optional>

namespace scuffedredis {

/**
 * High-level Redis client.
 * 
 * Encapsulates protocol handling and provides
 * convenient methods for Redis operations.
 */
class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    
    /**
     * Connect to Redis server.
     */
    bool connect(const std::string& host = "127.0.0.1", 
                 uint16_t port = 6380, 
                 int timeout_ms = 5000);
    
    /**
     * Disconnect from server.
     */
    void disconnect();
    
    /**
     * Check connection status.
     */
    bool is_connected() const { return client_.is_connected(); }
    
    /**
     * Execute raw command.
     * Args: ["SET", "key", "value"] or ["GET", "key"]
     * Returns response as MessagePtr.
     */
    protocol::MessagePtr execute(const std::vector<std::string>& args);
    
    /**
     * Execute command and get string response.
     * Useful for simple commands.
     */
    std::optional<std::string> execute_string(const std::vector<std::string>& args);
    
    /**
     * Parse command line into arguments.
     * Handles quoted strings and escapes.
     */
    static std::vector<std::string> parse_command_line(const std::string& line);
    
    /**
     * Format response for display.
     * Converts protocol message to human-readable format.
     */
    static std::string format_response(const protocol::MessagePtr& response);
    
    // Convenience methods for common operations
    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    std::vector<std::string> keys(const std::string& pattern = "*");
    bool ping();
    
private:
    TcpClient client_;              // TCP connection
    protocol::Parser parser_;       // Protocol parser
    
    /**
     * Send command and receive response.
     * Handles protocol serialization and parsing.
     */
    protocol::MessagePtr send_command(const protocol::MessagePtr& cmd);
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_REDIS_CLIENT_HPP
