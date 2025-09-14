#include "socket.hpp"
#include "../utils/logger.hpp"
#include <cstring>
#include <stdexcept>

namespace scuffedredis {
namespace network {

Socket::Socket() : fd_(-1) {
    createSocket();
}

Socket::Socket(int fd) : fd_(fd) {
    // Basically just a wrapper around existing file descriptor
}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

bool Socket::createSocket() {
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
        utils::Logger::error("Failed to create socket: {}", strerror(errno));
        return false;
    }
    return true;
}

bool Socket::bind(const std::string& host, int port) {
    if (!isValid()) {
        utils::Logger::error("Cannot bind invalid socket");
        return false;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (host.empty() || host == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            utils::Logger::error("Invalid host address: {}", host);
            return false;
        }
    }
    
    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        utils::Logger::error("Failed to bind socket: {}", strerror(errno));
        return false;
    }
    
    utils::Logger::info("Socket bound to {}:{}", host, port);
    return true;
}

bool Socket::listen(int backlog) {
    if (::listen(fd_, backlog) < 0) {
        utils::Logger::error("Failed to listen on socket: {}", strerror(errno));
        return false;
    }
    utils::Logger::info("Socket listening with backlog: {}", backlog);
    return true;
}

std::unique_ptr<Socket> Socket::accept() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = ::accept(fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            utils::Logger::error("Failed to accept connection: {}", strerror(errno));
        }
        return nullptr;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    utils::Logger::debug("Accepted connection from {}:{}", 
                        client_ip, ntohs(client_addr.sin_port));
    
    return std::make_unique<Socket>(client_fd);
}

bool Socket::connect(const std::string& host, int port) {
    if (!isValid()) {
        utils::Logger::error("Cannot connect with invalid socket");
        return false;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        utils::Logger::error("Invalid host address: {}", host);
        return false;
    }
    
    if (::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        utils::Logger::error("Failed to connect to {}:{} - {}", host, port, strerror(errno));
        return false;
    }
    
    utils::Logger::info("Connected to {}:{}", host, port);
    return true;
}

ssize_t Socket::send(const void* data, size_t len) {
    if (!isValid()) {
        return -1;
    }
    
    ssize_t sent = ::send(fd_, data, len, MSG_NOSIGNAL);
    if (sent < 0) {
        utils::Logger::error("Send failed: {}", strerror(errno));
    }
    return sent;
}

ssize_t Socket::recv(void* buffer, size_t len) {
    if (!isValid()) {
        return -1;
    }
    
    ssize_t received = ::recv(fd_, buffer, len, 0);
    if (received < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            utils::Logger::error("Recv failed: {}", strerror(errno));
        }
    }
    return received;
}

bool Socket::setNonBlocking(bool nonBlocking) {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0) {
        utils::Logger::error("Failed to get socket flags: {}", strerror(errno));
        return false;
    }
    
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    if (fcntl(fd_, F_SETFL, flags) < 0) {
        utils::Logger::error("Failed to set socket flags: {}", strerror(errno));
        return false;
    }
    
    return true;
}

bool Socket::setReuseAddr(bool reuse) {
    int opt = reuse ? 1 : 0;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        utils::Logger::error("Failed to set SO_REUSEADDR: {}", strerror(errno));
        return false;
    }
    return true;
}

bool Socket::setKeepAlive(bool keepAlive) {
    int opt = keepAlive ? 1 : 0;
    if (setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
        utils::Logger::error("Failed to set SO_KEEPALIVE: {}", strerror(errno));
        return false;
    }
    return true;
}

void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

} // namespace network
} // namespace scuffedredis
