#ifndef SCUFFEDREDIS_TCP_CLIENT_HPP
#define SCUFFEDREDIS_TCP_CLIENT_HPP

/**
 * TCP Client implementation for Redis.
 * 
 * Provides connection management and request/response handling.
 * Used by the CLI client and for testing the server.
 */

#include "socket.hpp"
#include <vector>
#include <string>
#include <chrono>

namespace scuffedredis {

/**
 * TCP Client for connecting to Redis server.
 */
class TcpClient {
public:
    TcpClient();
    ~TcpClient();
    
    /**
     * Connect to server.
     * address: Server IP address
     * port: Server port
     * timeout_ms: Connection timeout in milliseconds (0 for no timeout)
     */
    bool connect(const std::string& address, uint16_t port, int timeout_ms = 5000);
    
    /**
     * Disconnect from server.
     */
    void disconnect();
    
    /**
     * Check if connected to server.
     */
    bool is_connected() const { return socket_.is_valid() && connected_; }
    
    /**
     * Send raw data to server.
     * Returns true on success, false on failure.
     */
    bool send_raw(const void* data, size_t size);
    
    /**
     * Send string data to server.
     */
    bool send_string(const std::string& str);
    
    /**
     * Send command to server.
     * Will be enhanced with protocol encoding later.
     */
    bool send_command(const std::string& cmd);
    
    /**
     * Receive raw data from server.
     * Returns number of bytes received, 0 on close, -1 on error.
     */
    ssize_t receive_raw(void* buffer, size_t size);
    
    /**
     * Receive data with timeout.
     * Returns number of bytes received, 0 on timeout/close, -1 on error.
     */
    ssize_t receive_with_timeout(void* buffer, size_t size, int timeout_ms);
    
    /**
     * Read response from server.
     * Reads until complete response is received (protocol dependent).
     * For now, reads until newline or buffer full.
     */
    std::string read_response(int timeout_ms = 5000);
    
    /**
     * Send command and wait for response.
     * Convenience method combining send and receive.
     */
    std::string send_and_receive(const std::string& cmd, int timeout_ms = 5000);
    
    /**
     * Get server address info.
     */
    std::string get_server_info() const { return server_info_; }
    
    /**
     * Set read buffer size.
     * Larger buffers may improve performance for bulk operations.
     */
    void set_buffer_size(size_t size);

private:
    Socket socket_;                    // Client socket
    bool connected_;                   // Connection state
    std::string server_info_;         // Server address:port
    std::vector<uint8_t> read_buffer_; // Internal read buffer
    
    // Buffer configuration
    size_t buffer_size_;
    static constexpr size_t DEFAULT_BUFFER_SIZE = 4096;
    static constexpr size_t MAX_BUFFER_SIZE = 1024 * 1024;  // 1MB max
    
    /**
     * Wait for socket to be ready for reading.
     * Returns true if ready, false on timeout or error.
     */
    bool wait_for_data(int timeout_ms);
    
    /**
     * Initialize client state.
     */
    void init();
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_TCP_CLIENT_HPP
