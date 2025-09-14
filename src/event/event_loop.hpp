#ifndef SCUFFEDREDIS_EVENT_LOOP_HPP
#define SCUFFEDREDIS_EVENT_LOOP_HPP

/**
 * Event Loop for ScuffedRedis.
 * 
 * Provides event-driven I/O using select/poll for cross-platform support.
 * Handles multiple client connections efficiently.
 */

#include "network/socket.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

namespace scuffedredis {

// Forward declarations
class ClientConnection;

/**
 * Event types for the event loop.
 */
enum class EventType {
    READ = 1,    // Socket is ready for reading
    WRITE = 2,   // Socket is ready for writing
    ERROR = 4    // Socket has an error
};

/**
 * Event callback function type.
 * Called when an event occurs on a socket.
 */
using EventCallback = std::function<void(socket_t fd, EventType event)>;

/**
 * Connection manager for tracking client connections.
 */
class ConnectionManager {
public:
    ConnectionManager();
    ~ConnectionManager();
    
    /**
     * Add a new client connection.
     * Returns connection ID for tracking.
     */
    uint64_t add_connection(std::unique_ptr<ClientConnection> conn);
    
    /**
     * Remove a connection by ID.
     */
    void remove_connection(uint64_t conn_id);
    
    /**
     * Get connection by ID.
     */
    ClientConnection* get_connection(uint64_t conn_id);
    
    /**
     * Get all active connection IDs.
     */
    std::vector<uint64_t> get_connection_ids() const;
    
    /**
     * Get number of active connections.
     */
    size_t size() const { return connections_.size(); }
    
    /**
     * Clear all connections.
     */
    void clear();

private:
    std::unordered_map<uint64_t, std::unique_ptr<ClientConnection>> connections_;
    std::atomic<uint64_t> next_conn_id_;
    mutable std::mutex mutex_;
};

/**
 * Cross-platform event loop using select/poll.
 * 
 * Features:
 * - Non-blocking I/O for all sockets
 * - Efficient multiplexing with select/poll
 * - Connection management
 * - Event callbacks
 * - Thread-safe operations
 */
class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    
    /**
     * Start the event loop.
     * Runs until stop() is called.
     */
    void run();
    
    /**
     * Stop the event loop.
     * Thread-safe, can be called from any thread.
     */
    void stop();
    
    /**
     * Check if the event loop is running.
     */
    bool is_running() const { return running_.load(); }
    
    /**
     * Add a socket to the event loop.
     * fd: Socket file descriptor
     * events: Bitmask of EventType flags
     * callback: Function to call when events occur
     */
    void add_socket(socket_t fd, int events, EventCallback callback);
    
    /**
     * Remove a socket from the event loop.
     */
    void remove_socket(socket_t fd);
    
    /**
     * Update events for a socket.
     * Used to change which events we're interested in.
     */
    void update_socket(socket_t fd, int events);
    
    /**
     * Add a client connection.
     * Returns connection ID.
     */
    uint64_t add_client(std::unique_ptr<ClientConnection> conn);
    
    /**
     * Remove a client connection.
     */
    void remove_client(uint64_t conn_id);
    
    /**
     * Get connection manager.
     */
    ConnectionManager& get_connections() { return connections_; }
    
    /**
     * Get event loop statistics.
     */
    struct Stats {
        size_t active_connections;
        size_t events_processed;
        size_t sockets_monitored;
        double events_per_second;
    };
    
    Stats get_stats() const;

private:
    // Event loop state
    std::atomic<bool> running_;
    std::atomic<bool> stop_requested_;
    std::thread event_thread_;
    
    // Socket management
    std::unordered_map<socket_t, EventCallback> socket_callbacks_;
    std::unordered_map<socket_t, int> socket_events_;
    mutable std::mutex socket_mutex_;
    
    // Connection management
    ConnectionManager connections_;
    
    // Statistics
    std::atomic<size_t> events_processed_;
    std::chrono::steady_clock::time_point start_time_;
    
    /**
     * Main event loop function.
     * Runs in separate thread.
     */
    void event_loop_main();
    
    /**
     * Process events using select().
     * Returns number of events processed.
     */
    int process_events(int timeout_ms);
    
    /**
     * Setup file descriptor sets for select().
     */
    void setup_fd_sets(fd_set& read_fds, fd_set& write_fds, 
                      fd_set& error_fds, int& max_fd);
    
    /**
     * Process events from select().
     */
    void process_select_events(const fd_set& read_fds, 
                              const fd_set& write_fds,
                              const fd_set& error_fds);
    
    /**
     * Handle new client connection.
     */
    void handle_new_connection(socket_t listen_fd);
    
    /**
     * Handle client data.
     */
    void handle_client_data(socket_t client_fd);
    
    /**
     * Handle client write ready.
     */
    void handle_client_write(socket_t client_fd);
    
    /**
     * Handle client error.
     */
    void handle_client_error(socket_t client_fd);
};

/**
 * Global event loop instance.
 * Singleton for easy access throughout the application.
 */
class EventLoopManager {
public:
    static EventLoopManager& instance() {
        static EventLoopManager manager;
        return manager;
    }
    
    EventLoop& get_loop() { return loop_; }
    
    /**
     * Start the global event loop.
     */
    void start() {
        if (!loop_.is_running()) {
            loop_.run();
        }
    }
    
    /**
     * Stop the global event loop.
     */
    void stop() {
        loop_.stop();
    }

private:
    EventLoopManager() = default;
    EventLoop loop_;
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_EVENT_LOOP_HPP
