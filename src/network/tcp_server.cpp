#include "tcp_server.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <chrono>

namespace scuffedredis {

// ============================================================================
// ClientConnection Implementation
// ============================================================================

ClientConnection::ClientConnection(Socket&& socket) 
    : socket_(std::move(socket)), 
      closed_(false) {
    // Reserve initial buffer space for efficiency
    read_buffer_.reserve(READ_BUFFER_SIZE);
    
    // TODO: Get client address info for logging
    client_info_ = "client";  // Placeholder
}

ClientConnection::~ClientConnection() {
    close();
}

ssize_t ClientConnection::read() {
    if (!is_connected()) return -1;
    
    // Temporary buffer for reading
    uint8_t temp_buffer[READ_BUFFER_SIZE];
    
    // Read from socket
    ssize_t bytes_read = socket_.recv(temp_buffer, sizeof(temp_buffer));
    
    if (bytes_read > 0) {
        // Check buffer size limit to prevent memory exhaustion
        if (read_buffer_.size() + bytes_read > MAX_BUFFER_SIZE) {
            std::cerr << "Client buffer overflow, closing connection" << std::endl;
            close();
            return -1;
        }
        
        // Append to read buffer
        read_buffer_.insert(read_buffer_.end(), 
                          temp_buffer, 
                          temp_buffer + bytes_read);
    } else if (bytes_read == 0) {
        // Connection closed by client
        close();
    }
    // bytes_read < 0 means error (could be EAGAIN for non-blocking)
    
    return bytes_read;
}

bool ClientConnection::write(const void* data, size_t size) {
    if (!is_connected() || size == 0) return false;
    
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    size_t total_sent = 0;
    
    // Keep sending until all data is sent
    // Handles partial writes automatically
    while (total_sent < size) {
        ssize_t sent = socket_.send(bytes + total_sent, size - total_sent);
        
        if (sent < 0) {
            std::cerr << "Failed to write to client: " << socket_.get_last_error() << std::endl;
            close();
            return false;
        }
        
        total_sent += sent;
    }
    
    return true;
}

bool ClientConnection::write(const std::string& str) {
    return write(str.data(), str.size());
}

void ClientConnection::consume_bytes(size_t count) {
    if (count >= read_buffer_.size()) {
        read_buffer_.clear();
    } else {
        // Erase consumed bytes from beginning of buffer
        read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + count);
    }
}

void ClientConnection::close() {
    if (!closed_) {
        socket_.close();
        closed_ = true;
        read_buffer_.clear();
        write_buffer_.clear();
    }
}

// ============================================================================
// TcpServer Implementation
// ============================================================================

TcpServer::TcpServer() 
    : running_(false), 
      stop_requested_(false),
      port_(0) {
}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::init(const std::string& address, uint16_t port) {
    // Initialize sockets (Windows specific)
    if (!initialize_sockets()) {
        return false;
    }
    
    // Create TCP socket
    if (!listen_socket_.create_tcp()) {
        return false;
    }
    
    // Enable address reuse to avoid TIME_WAIT issues
    listen_socket_.set_reuseaddr(true);
    
    // Bind to address and port
    if (!listen_socket_.bind(address, port)) {
        return false;
    }
    
    // Start listening for connections
    if (!listen_socket_.listen(128)) {  // 128 connection backlog
        return false;
    }
    
    bind_address_ = address;
    port_ = port;
    
    std::cout << "Server initialized on " << address << ":" << port << std::endl;
    return true;
}

void TcpServer::run_blocking(ClientHandler handler) {
    if (!listen_socket_.is_valid()) {
        std::cerr << "Server not initialized" << std::endl;
        return;
    }
    
    running_ = true;
    stop_requested_ = false;
    
    std::cout << "Server running in blocking mode..." << std::endl;
    
    // Main accept loop
    while (running_ && !stop_requested_) {
        // Accept new connection
        Socket client_socket = listen_socket_.accept();
        
        if (!client_socket.is_valid()) {
            // Could be due to non-blocking mode or actual error
            continue;
        }
        
        std::cout << "New client connected" << std::endl;
        
        // Disable Nagle's algorithm for low latency
        client_socket.set_nodelay(true);
        
        // Create client connection object
        auto client = std::make_unique<ClientConnection>(std::move(client_socket));
        
        // Handle client in main thread (for now)
        // Will be improved with event loop and threading later
        handle_client(std::move(client), handler);
        
        // Clean up closed connections periodically
        cleanup_connections();
    }
    
    running_ = false;
    std::cout << "Server stopped" << std::endl;
}

void TcpServer::run_async(ClientHandler handler) {
    if (!listen_socket_.is_valid()) {
        std::cerr << "Server not initialized" << std::endl;
        return;
    }
    
    // Start server in separate thread
    server_thread_ = std::thread([this, handler]() {
        run_blocking(handler);
    });
    
    std::cout << "Server running in async mode..." << std::endl;
}

void TcpServer::stop() {
    stop_requested_ = true;
    
    // Close listening socket to break accept() call
    listen_socket_.close();
    
    // Wait for server thread if running async
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    // Close all client connections
    for (auto& conn : connections_) {
        if (conn) {
            conn->close();
        }
    }
    connections_.clear();
    
    running_ = false;
    
    // Cleanup sockets (Windows specific)
    cleanup_sockets();
}

std::string TcpServer::get_server_info() const {
    return bind_address_ + ":" + std::to_string(port_);
}

void TcpServer::handle_client(std::unique_ptr<ClientConnection> client, 
                              ClientHandler handler) {
    // Simple blocking handler for now
    // Will be replaced with event-driven handling later
    
    while (client->is_connected()) {
        // Read data from client
        ssize_t bytes_read = client->read();
        
        if (bytes_read <= 0) {
            // Error or connection closed
            break;
        }
        
        // Process client data with handler
        if (!handler(*client)) {
            // Handler requested connection close
            break;
        }
    }
    
    std::cout << "Client disconnected" << std::endl;
    client->close();
}

void TcpServer::cleanup_connections() {
    // Remove closed connections from the list
    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
                      [](const std::unique_ptr<ClientConnection>& conn) {
                          return !conn || !conn->is_connected();
                      }),
        connections_.end()
    );
}
