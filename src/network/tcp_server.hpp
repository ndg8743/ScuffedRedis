#ifndef SCUFFEDREDIS_TCP_SERVER_HPP
#define SCUFFEDREDIS_TCP_SERVER_HPP

/**
 * TCP Server implementation for Redis.
 * 
 * Handles incoming connections and manages client sessions.
 * Will evolve to support event-driven I/O and concurrent connections.
 */

#include "socket.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

namespace scuffedredis {

// Forward declaration
class ClientConnection;

/**
 * Callback type for handling client data.
 * Returns true to keep connection open, false to close it.
 */
using ClientHandler = std::function<bool(ClientConnection&)>;

/**
 * Represents a single client connection.
 * Manages the socket and buffered I/O for a client.
 */
class ClientConnection {
public:
    explicit ClientConnection(Socket&& socket);
    ~ClientConnection();
    
    /**
     * Read data from client into internal buffer.
     * Returns number of bytes read, 0 on close, -1 on error.
     */
    ssize_t read();
    
    /**
     * Write data to client.
     * Handles partial writes automatically.
     */
    bool write(const void* data, size_t size);
    bool write(const std::string& str);
    
    /**
     * Get data from read buffer.
     * Does not remove data from buffer.
     */
    const std::vector<uint8_t>& get_read_buffer() const { return read_buffer_; }
    
    /**
     * Consume bytes from read buffer.
     * Used after processing data to remove it from buffer.
     */
    void consume_bytes(size_t count);
    
    /**
     * Check if connection is still valid.
     */
    bool is_connected() const { return socket_.is_valid() && !closed_; }
    
    /**
     * Close the connection.
     */
    void close();
    
    /**
     * Get client address information.
     */
    std::string get_client_info() const { return client_info_; }
    
    Socket& get_socket() { return socket_; }

private:
    Socket socket_;
    std::vector<uint8_t> read_buffer_;   // Buffer for incoming data
    std::vector<uint8_t> write_buffer_;  // Buffer for outgoing data (if needed)
    std::string client_info_;            // Client address:port string
    bool closed_;                        // Connection state
    
    // Buffer management constants
    static constexpr size_t READ_BUFFER_SIZE = 4096;
    static constexpr size_t MAX_BUFFER_SIZE = 1024 * 1024;  // 1MB max
};

/**
 * TCP Server for handling Redis connections.
 */
class TcpServer {
public:
    TcpServer();
    ~TcpServer();
    
    /**
     * Initialize server on specified address and port.
     * address: IP to bind to ("0.0.0.0" for all interfaces)
     * port: Port to listen on
     */
    bool init(const std::string& address, uint16_t port);
    
    /**
     * Run the server with blocking accept loop.
     * This is the simple version - will be replaced with event loop later.
     * handler: Callback function to process client connections
     */
    void run_blocking(ClientHandler handler);
    
    /**
     * Run server in a separate thread.
     * Non-blocking call that starts server in background.
     */
    void run_async(ClientHandler handler);
    
    /**
     * Stop the server.
     * Closes all connections and stops accept loop.
     */
    void stop();
    
    /**
     * Check if server is running.
     */
    bool is_running() const { return running_.load(); }
    
    /**
     * Get server address and port.
     */
    std::string get_server_info() const;
    
    /**
     * Get number of active connections.
     */
    size_t get_connection_count() const { return connections_.size(); }

private:
    Socket listen_socket_;                              // Server listening socket
    std::vector<std::unique_ptr<ClientConnection>> connections_;  // Active connections
    std::atomic<bool> running_;                        // Server state
    std::atomic<bool> stop_requested_;                 // Shutdown flag
    std::thread server_thread_;                        // Async server thread
    std::string bind_address_;                         // Server bind address
    uint16_t port_;                                    // Server port
    
    /**
     * Accept new connections.
     * Called in accept loop.
     */
    void accept_connections();
    
    /**
     * Handle a single client connection.
     * Runs in the main thread for now, will be threaded later.
     */
    void handle_client(std::unique_ptr<ClientConnection> client, ClientHandler handler);
    
    /**
     * Remove closed connections from the list.
     */
    void cleanup_connections();
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_TCP_SERVER_HPP
