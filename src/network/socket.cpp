#include "socket.hpp"
#include <cstring>
#include <iostream>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // Used to get more detailed error messages on Windows
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <fcntl.h>
    #include <netinet/tcp.h>
    #include <errno.h>
#endif

namespace scuffedredis {

// Platform-specific initialization
bool initialize_sockets() {
#ifdef _WIN32
    WSADATA wsa_data;
    // Initialize Winsock version 2.2
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
#endif
    return true;
}

void cleanup_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Helper function to get platform-specific error message
std::string Socket::get_socket_error() {
#ifdef _WIN32
    int error = WSAGetLastError();
    char msg_buf[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   msg_buf, sizeof(msg_buf), NULL);
    return std::string(msg_buf);
#else
    return std::string(strerror(errno));
#endif
}

Socket::Socket() : fd_(INVALID_SOCKET_FD) {}

Socket::Socket(socket_t fd) : fd_(fd) {}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = INVALID_SOCKET_FD;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = INVALID_SOCKET_FD;
    }
    return *this;
}

bool Socket::create_tcp() {
    // Close existing socket if any
    close();
    
    // Create TCP socket (AF_INET for IPv4, SOCK_STREAM for TCP)
    fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd_ == INVALID_SOCKET_FD) {
        std::cerr << "Failed to create socket: " << get_socket_error() << std::endl;
        return false;
    }
    
    return true;
}

bool Socket::bind(const std::string& address, uint16_t port) {
    if (!is_valid()) return false;
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);  // Convert to network byte order
    
    // Convert IP address string to binary form
    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << address << std::endl;
        return false;
    }
    
    // Bind socket to address
    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Failed to bind to " << address << ":" << port 
                  << " - " << get_socket_error() << std::endl;
        return false;
    }
    
    return true;
}

bool Socket::listen(int backlog) {
    if (!is_valid()) return false;
    
    if (::listen(fd_, backlog) < 0) {
        std::cerr << "Failed to listen: " << get_socket_error() << std::endl;
        return false;
    }
    
    return true;
}

Socket Socket::accept() {
    if (!is_valid()) return Socket();
    
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    
    // Accept incoming connection
    socket_t client_fd = ::accept(fd_, 
                                  reinterpret_cast<sockaddr*>(&client_addr), 
                                  &addr_len);
    
    if (client_fd == INVALID_SOCKET_FD) {
        // Not necessarily an error - could be non-blocking with no connections
        return Socket();
    }
    
    // Create new Socket object for the client connection
    return Socket(client_fd);
}

bool Socket::connect(const std::string& address, uint16_t port) {
    if (!is_valid()) return false;
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert IP address string to binary form
    if (inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << address << std::endl;
        return false;
    }
    
    // Connect to server
    if (::connect(fd_, reinterpret_cast<sockaddr*>(&server_addr), 
                  sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to " << address << ":" << port 
                  << " - " << get_socket_error() << std::endl;
        return false;
    }
    
    return true;
}

ssize_t Socket::send(const void* data, size_t size) {
    if (!is_valid()) return -1;
    
#ifdef _WIN32
    // Windows send returns int
    int result = ::send(fd_, static_cast<const char*>(data), 
                        static_cast<int>(size), 0);
    return static_cast<ssize_t>(result);
#else
    // Unix send returns ssize_t
    return ::send(fd_, data, size, 0);
#endif
}

ssize_t Socket::recv(void* buffer, size_t size) {
    if (!is_valid()) return -1;
    
#ifdef _WIN32
    // Windows recv returns int
    int result = ::recv(fd_, static_cast<char*>(buffer), 
                        static_cast<int>(size), 0);
    return static_cast<ssize_t>(result);
#else
    // Unix recv returns ssize_t
    return ::recv(fd_, buffer, size, 0);
#endif
}

bool Socket::set_nonblocking(bool enable) {
    if (!is_valid()) return false;
    
#ifdef _WIN32
    // Windows uses ioctlsocket
    u_long mode = enable ? 1 : 0;
    if (ioctlsocket(fd_, FIONBIO, &mode) != 0) {
        std::cerr << "Failed to set non-blocking mode: " 
                  << get_socket_error() << std::endl;
        return false;
    }
#else
    // Unix uses fcntl
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "Failed to get socket flags: " 
                  << get_socket_error() << std::endl;
        return false;
    }
    
    if (enable) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    if (fcntl(fd_, F_SETFL, flags) == -1) {
        std::cerr << "Failed to set non-blocking mode: " 
                  << get_socket_error() << std::endl;
        return false;
    }
#endif
    
    return true;
}

bool Socket::set_nodelay(bool enable) {
    if (!is_valid()) return false;
    
    // TCP_NODELAY disables Nagle's algorithm for lower latency
    int flag = enable ? 1 : 0;
    if (setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, 
                   reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
        std::cerr << "Failed to set TCP_NODELAY: " 
                  << get_socket_error() << std::endl;
        return false;
    }
    
    return true;
}

bool Socket::set_reuseaddr(bool enable) {
    if (!is_valid()) return false;
    
    // SO_REUSEADDR allows binding to recently used addresses
    // Useful for server restart without waiting for TIME_WAIT
    int flag = enable ? 1 : 0;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, 
                   reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " 
                  << get_socket_error() << std::endl;
        return false;
    }
    
    return true;
}

void Socket::close() {
    if (is_valid()) {
#ifdef _WIN32
        closesocket(fd_);
#else
        ::close(fd_);
#endif
        fd_ = INVALID_SOCKET_FD;
    }
}

std::string Socket::get_last_error() const {
    return get_socket_error();
}

} // namespace scuffedredis
