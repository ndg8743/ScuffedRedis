#include "tcp_client.hpp"
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/select.h>
    #include <sys/time.h>
#endif

namespace scuffedredis {

TcpClient::TcpClient() {
    init();
}

TcpClient::~TcpClient() {
    disconnect();
}

void TcpClient::init() {
    connected_ = false;
    buffer_size_ = DEFAULT_BUFFER_SIZE;
    read_buffer_.reserve(buffer_size_);
    
    // Initialize sockets (Windows specific)
    initialize_sockets();
}

bool TcpClient::connect(const std::string& address, uint16_t port, int timeout_ms) {
    // Close existing connection if any
    disconnect();
    
    // Create TCP socket
    if (!socket_.create_tcp()) {
        return false;
    }
    
    // Store server info for later reference
    server_info_ = address + ":" + std::to_string(port);
    
    // For timeout support, we need non-blocking mode
    if (timeout_ms > 0) {
        socket_.set_nonblocking(true);
    }
    
    // Attempt connection
    bool connect_result = socket_.connect(address, port);
    
    if (!connect_result && timeout_ms > 0) {
        // For non-blocking connect, we need to wait for completion
        // This would normally check for write readiness with select()
        // Simplified version for now
        connect_result = wait_for_data(timeout_ms);
    }
    
    if (connect_result || timeout_ms == 0) {
        // Connection successful or blocking mode
        connected_ = socket_.is_valid();
        
        if (connected_) {
            // Set socket options for optimal performance
            socket_.set_nodelay(true);  // Disable Nagle for low latency
            
            // Return to blocking mode for simplicity
            if (timeout_ms > 0) {
                socket_.set_nonblocking(false);
            }
            
            std::cout << "Connected to " << server_info_ << std::endl;
        }
    }
    
    return connected_;
}

void TcpClient::disconnect() {
    if (connected_) {
        socket_.close();
        connected_ = false;
        read_buffer_.clear();
        std::cout << "Disconnected from " << server_info_ << std::endl;
    }
}

bool TcpClient::send_raw(const void* data, size_t size) {
    if (!is_connected()) return false;
    
    // Send all data, handling partial sends
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    size_t total_sent = 0;
    
    while (total_sent < size) {
        ssize_t sent = socket_.send(bytes + total_sent, size - total_sent);
        
        if (sent <= 0) {
            std::cerr << "Send failed: " << socket_.get_last_error() << std::endl;
            disconnect();
            return false;
        }
        
        total_sent += sent;
    }
    
    return true;
}

bool TcpClient::send_string(const std::string& str) {
    return send_raw(str.data(), str.size());
}

bool TcpClient::send_command(const std::string& cmd) {
    // For now, just send raw command
    // Will be enhanced with protocol encoding later
    std::string formatted = cmd;
    
    // Add newline if not present (simple text protocol for testing)
    if (!formatted.empty() && formatted.back() != '\n') {
        formatted += "\r\n";
    }
    
    return send_string(formatted);
}

ssize_t TcpClient::receive_raw(void* buffer, size_t size) {
    if (!is_connected()) return -1;
    
    ssize_t received = socket_.recv(buffer, size);
    
    if (received == 0) {
        // Connection closed by server
        disconnect();
    } else if (received < 0) {
        // Check if it's a real error or just EAGAIN/EWOULDBLOCK
        std::cerr << "Receive error: " << socket_.get_last_error() << std::endl;
        disconnect();
    }
    
    return received;
}

ssize_t TcpClient::receive_with_timeout(void* buffer, size_t size, int timeout_ms) {
    if (!is_connected()) return -1;
    
    // Wait for data to be available
    if (!wait_for_data(timeout_ms)) {
        return 0;  // Timeout
    }
    
    // Data is available, receive it
    return receive_raw(buffer, size);
}

bool TcpClient::wait_for_data(int timeout_ms) {
    if (!socket_.is_valid()) return false;
    
    // Use select() to wait for data with timeout
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket_.get_fd(), &read_fds);
    
    // Setup timeout
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
    // Wait for socket to be ready
    // select() modifies the timeout on some platforms, so use a copy
    int result = select(static_cast<int>(socket_.get_fd()) + 1, 
                       &read_fds, nullptr, nullptr, 
                       timeout_ms > 0 ? &tv : nullptr);
    
    return result > 0;
}

std::string TcpClient::read_response(int timeout_ms) {
    if (!is_connected()) return "";
    
    read_buffer_.clear();
    
    // Read until we get a complete response
    // For now, read until newline or timeout
    auto start_time = std::chrono::steady_clock::now();
    
    while (true) {
        // Check timeout
        if (timeout_ms > 0) {
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            if (elapsed_ms >= timeout_ms) {
                break;  // Timeout
            }
        }
        
        // Read more data
        uint8_t temp_buffer[256];
        ssize_t received = receive_with_timeout(temp_buffer, sizeof(temp_buffer), 100);
        
        if (received > 0) {
            // Append to buffer
            read_buffer_.insert(read_buffer_.end(), 
                              temp_buffer, 
                              temp_buffer + received);
            
            // Check for complete response (newline for now)
            // Will be replaced with proper protocol parsing later
            auto it = std::find(read_buffer_.begin(), read_buffer_.end(), '\n');
            if (it != read_buffer_.end()) {
                // Found complete response
                return std::string(read_buffer_.begin(), it + 1);
            }
            
            // Prevent buffer overflow
            if (read_buffer_.size() > MAX_BUFFER_SIZE) {
                std::cerr << "Response too large" << std::endl;
                break;
            }
        } else if (received == 0) {
            // Timeout or connection closed
            break;
        } else {
            // Error
            break;
        }
    }
    
    // Return whatever we have
    return std::string(read_buffer_.begin(), read_buffer_.end());
}

std::string TcpClient::send_and_receive(const std::string& cmd, int timeout_ms) {
    if (!send_command(cmd)) {
        return "";
    }
    
    return read_response(timeout_ms);
}

void TcpClient::set_buffer_size(size_t size) {
    if (size > 0 && size <= MAX_BUFFER_SIZE) {
        buffer_size_ = size;
        read_buffer_.reserve(buffer_size_);
    }
}

} // namespace scuffedredis
