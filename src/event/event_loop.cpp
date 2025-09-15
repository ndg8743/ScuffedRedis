#include "event_loop.hpp"
#include "network/tcp_server.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/select.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif

namespace scuffedredis {

// ============================================================================
// ConnectionManager Implementation
// ============================================================================

ConnectionManager::ConnectionManager() : next_conn_id_(1) {
}

ConnectionManager::~ConnectionManager() {
    clear();
}

uint64_t ConnectionManager::add_connection(std::unique_ptr<ClientConnection> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t conn_id = next_conn_id_++;
    connections_[conn_id] = std::move(conn);
    
    LOG_DEBUG(format_log("Added connection ", conn_id, 
                        " (total: ", connections_.size(), ")"));
    
    return conn_id;
}

void ConnectionManager::remove_connection(uint64_t conn_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = connections_.find(conn_id);
    if (it != connections_.end()) {
        connections_.erase(it);
        LOG_DEBUG(format_log("Removed connection ", conn_id, 
                            " (total: ", connections_.size(), ")"));
    }
}

ClientConnection* ConnectionManager::get_connection(uint64_t conn_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = connections_.find(conn_id);
    return (it != connections_.end()) ? it->second.get() : nullptr;
}

std::vector<uint64_t> ConnectionManager::get_connection_ids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<uint64_t> ids;
    ids.reserve(connections_.size());
    
    for (const auto& pair : connections_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void ConnectionManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.clear();
}

// ============================================================================
// EventLoop Implementation
// ============================================================================

EventLoop::EventLoop() 
    : running_(false), 
      stop_requested_(false),
      events_processed_(0),
      start_time_(std::chrono::steady_clock::now()) {
}

EventLoop::~EventLoop() {
    stop();
}

void EventLoop::run() {
    if (running_.load()) {
        LOG_WARN("Event loop is already running");
        return;
    }
    
    running_ = true;
    stop_requested_ = false;
    start_time_ = std::chrono::steady_clock::now();
    
    LOG_INFO("Starting event loop");
    
    // Run event loop in current thread
    // In production, this would typically run in a separate thread
    event_loop_main();
    
    running_ = false;
    LOG_INFO("Event loop stopped");
}

void EventLoop::stop() {
    if (!running_.load()) {
        return;
    }
    
    LOG_INFO("Stopping event loop...");
    stop_requested_ = true;
    
    // Wait for event loop to finish
    if (event_thread_.joinable()) {
        event_thread_.join();
    }
}

void EventLoop::add_socket(socket_t fd, int events, EventCallback callback) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    socket_callbacks_[fd] = callback;
    socket_events_[fd] = events;
    
    LOG_DEBUG(format_log("Added socket ", fd, " with events ", events));
}

void EventLoop::remove_socket(socket_t fd) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    socket_callbacks_.erase(fd);
    socket_events_.erase(fd);
    
    LOG_DEBUG(format_log("Removed socket ", fd));
}

void EventLoop::update_socket(socket_t fd, int events) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    auto it = socket_events_.find(fd);
    if (it != socket_events_.end()) {
        it->second = events;
        LOG_DEBUG(format_log("Updated socket ", fd, " events to ", events));
    }
}

uint64_t EventLoop::add_client(std::unique_ptr<ClientConnection> conn) {
    return connections_.add_connection(std::move(conn));
}

void EventLoop::remove_client(uint64_t conn_id) {
    connections_.remove_connection(conn_id);
}

void EventLoop::event_loop_main() {
    LOG_INFO("Event loop main thread started");
    
    while (!stop_requested_.load()) {
        // Process events with 100ms timeout
        int events = process_events(100);
        
        if (events < 0) {
            LOG_ERROR("Error in event processing");
            break;
        }
        
        events_processed_ += events;
        
        // Clean up closed connections periodically
        if (events_processed_ % 100 == 0) {
            // TODO: Implement connection cleanup
        }
    }
    
    LOG_INFO("Event loop main thread exiting");
}

int EventLoop::process_events(int timeout_ms) {
    fd_set read_fds, write_fds, error_fds;
    int max_fd = 0;
    
    // Setup file descriptor sets
    setup_fd_sets(read_fds, write_fds, error_fds, max_fd);
    
    if (max_fd == 0) {
        // No sockets to monitor, sleep briefly
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
        return 0;
    }
    
    // Setup timeout
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
    // Wait for events
    int result = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &tv);
    
    if (result < 0) {
        // Error in select
        LOG_ERROR(format_log("select() error: ", strerror(errno)));
        return -1;
    }
    
    if (result == 0) {
        // Timeout
        return 0;
    }
    
    // Process events
    process_select_events(read_fds, write_fds, error_fds);
    
    return result;
}

void EventLoop::setup_fd_sets(fd_set& read_fds, fd_set& write_fds, 
                             fd_set& error_fds, int& max_fd) {
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);
    max_fd = 0;
    
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    for (const auto& pair : socket_events_) {
        socket_t fd = pair.first;
        int events = pair.second;
        
        if (events & static_cast<int>(EventType::READ)) {
            FD_SET(fd, &read_fds);
        }
        if (events & static_cast<int>(EventType::WRITE)) {
            FD_SET(fd, &write_fds);
        }
        if (events & static_cast<int>(EventType::ERROR_EVENT)) {
            FD_SET(fd, &error_fds);
        }
        
        max_fd = (std::max)(max_fd, static_cast<int>(fd));
    }
}

void EventLoop::process_select_events(const fd_set& read_fds, 
                                     const fd_set& write_fds,
                                     const fd_set& error_fds) {
    std::lock_guard<std::mutex> lock(socket_mutex_);
    
    for (const auto& pair : socket_callbacks_) {
        socket_t fd = pair.first;
        const EventCallback& callback = pair.second;
        
        // Check for read events
        if (FD_ISSET(fd, &read_fds)) {
            callback(fd, EventType::READ);
        }
        
        // Check for write events
        if (FD_ISSET(fd, &write_fds)) {
            callback(fd, EventType::WRITE);
        }
        
        // Check for error events
        if (FD_ISSET(fd, &error_fds)) {
            callback(fd, EventType::ERROR_EVENT);
        }
    }
}

void EventLoop::handle_new_connection(socket_t listen_fd) {
    // This would be implemented by the TCP server
    // when using the event loop
    LOG_DEBUG(format_log("New connection on socket ", listen_fd));
}

void EventLoop::handle_client_data(socket_t client_fd) {
    // Find connection by socket fd
    auto conn_ids = connections_.get_connection_ids();
    
    for (uint64_t conn_id : conn_ids) {
        auto* conn = connections_.get_connection(conn_id);
        if (conn && conn->get_socket().get_fd() == client_fd) {
            // Process client data
            ssize_t bytes_read = conn->read();
            
            if (bytes_read <= 0) {
                // Connection closed or error
                LOG_DEBUG(format_log("Client ", conn_id, " disconnected"));
                remove_client(conn_id);
                remove_socket(client_fd);
            }
            break;
        }
    }
}

void EventLoop::handle_client_write(socket_t client_fd) {
    // Handle write events for clients
    LOG_DEBUG(format_log("Write ready for socket ", client_fd));
}

void EventLoop::handle_client_error(socket_t client_fd) {
    // Handle errors for clients
    LOG_ERROR(format_log("Error on socket ", client_fd));
    
    // Remove the connection
    auto conn_ids = connections_.get_connection_ids();
    
    for (uint64_t conn_id : conn_ids) {
        auto* conn = connections_.get_connection(conn_id);
        if (conn && conn->get_socket().get_fd() == client_fd) {
            remove_client(conn_id);
            remove_socket(client_fd);
            break;
        }
    }
}

// TODO: Implement cleanup_closed_connections method

EventLoop::Stats EventLoop::get_stats() const {
    Stats stats;
    stats.active_connections = connections_.size();
    stats.events_processed = events_processed_.load();
    
    {
        std::lock_guard<std::mutex> lock(socket_mutex_);
        stats.sockets_monitored = socket_events_.size();
    }
    
    // Calculate events per second
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - start_time_).count();
    
    if (elapsed > 0) {
        stats.events_per_second = static_cast<double>(stats.events_processed) / elapsed;
    } else {
        stats.events_per_second = 0.0;
    }
    
    return stats;
}

} // namespace scuffedredis
