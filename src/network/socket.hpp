#ifndef SCUFFEDREDIS_SOCKET_HPP
#define SCUFFEDREDIS_SOCKET_HPP

/**
 * Cross-platform socket abstraction layer.
 * 
 * Handles platform differences between Windows (Winsock) and Unix sockets.
 * Provides RAII wrapper for socket file descriptors.
 */

#include <string>
#include <cstdint>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // Need to link with Ws2_32.lib
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    const socket_t INVALID_SOCKET_FD = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
    const socket_t INVALID_SOCKET_FD = -1;
#endif

namespace scuffedredis {

/**
 * Initialize socket subsystem (Windows specific).
 * Must be called before any socket operations on Windows.
 * No-op on Unix systems.
 */
bool initialize_sockets();

/**
 * Cleanup socket subsystem (Windows specific).
 * Should be called at program termination on Windows.
 * No-op on Unix systems.
 */
void cleanup_sockets();

/**
 * RAII wrapper for socket file descriptors.
 * Automatically closes socket on destruction.
 */
class Socket {
public:
    Socket();
    explicit Socket(socket_t fd);
    ~Socket();
    
    // Move semantics - sockets can't be copied but can be moved
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    
    // Delete copy operations
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    /**
     * Create a new TCP socket.
     * Returns true on success, false on failure.
     */
    bool create_tcp();
    
    /**
     * Bind socket to address and port.
     * address: IP address to bind to (e.g., "0.0.0.0" for all interfaces)
     * port: Port number to bind to
     */
    bool bind(const std::string& address, uint16_t port);
    
    /**
     * Start listening for incoming connections.
     * backlog: Maximum number of pending connections
     */
    bool listen(int backlog = 10);
    
    /**
     * Accept incoming connection.
     * Returns new Socket for the accepted connection.
     * Returns invalid socket on failure.
     */
    Socket accept();
    
    /**
     * Connect to remote server.
     * address: Server IP address
     * port: Server port
     */
    bool connect(const std::string& address, uint16_t port);
    
    /**
     * Send data over socket.
     * Returns number of bytes sent, or -1 on error.
     */
    ssize_t send(const void* data, size_t size);
    
    /**
     * Receive data from socket.
     * Returns number of bytes received, 0 on connection close, or -1 on error.
     */
    ssize_t recv(void* buffer, size_t size);
    
    /**
     * Set socket to non-blocking mode.
     * Needed for event-driven I/O later.
     */
    bool set_nonblocking(bool enable = true);
    
    /**
     * Enable/disable Nagle's algorithm.
     * Disabling reduces latency but may increase bandwidth usage.
     */
    bool set_nodelay(bool enable = true);
    
    /**
     * Enable SO_REUSEADDR option.
     * Allows binding to recently used addresses.
     */
    bool set_reuseaddr(bool enable = true);
    
    /**
     * Close the socket.
     * Called automatically by destructor.
     */
    void close();
    
    /**
     * Check if socket is valid.
     */
    bool is_valid() const { return fd_ != INVALID_SOCKET_FD; }
    
    /**
     * Get underlying socket descriptor.
     * Used when interfacing with select/epoll/poll.
     */
    socket_t get_fd() const { return fd_; }
    
    /**
     * Get last error message.
     */
    std::string get_last_error() const;

private:
    socket_t fd_;  // Socket file descriptor
    
    // Helper to get error string for current platform
    static std::string get_socket_error();
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_SOCKET_HPP
