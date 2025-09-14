#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <memory>

namespace scuffedredis {
namespace network {

// Basic socket wrapper for network operations
// Need this because raw socket operations are error-prone
class Socket {
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();
    
    // Non-copyable but movable for efficiency
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    
    // Basic operations
    bool bind(const std::string& host, int port);
    bool listen(int backlog = 128);
    std::unique_ptr<Socket> accept();
    bool connect(const std::string& host, int port);
    
    // I/O operations with error handling
    ssize_t send(const void* data, size_t len);
    ssize_t recv(void* buffer, size_t len);
    
    // Socket configuration
    bool setNonBlocking(bool nonBlocking = true);
    bool setReuseAddr(bool reuse = true);
    bool setKeepAlive(bool keepAlive = true);
    
    // Getters
    int getFd() const { return fd_; }
    bool isValid() const { return fd_ >= 0; }
    
    // Close the socket
    void close();

private:
    int fd_;
    
    // Helper to create socket
    bool createSocket();
};

} // namespace network
} // namespace scuffedredis
