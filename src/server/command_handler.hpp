#ifndef SCUFFEDREDIS_COMMAND_HANDLER_HPP
#define SCUFFEDREDIS_COMMAND_HANDLER_HPP

/**
 * Command handler for processing client requests.
 * 
 * Bridges the network layer with the KV store.
 * Handles protocol parsing and response serialization.
 */

#include "network/tcp_server.hpp"
#include "protocol/protocol.hpp"
#include "kv_store.hpp"
#include <atomic>

namespace scuffedredis {

/**
 * Handles client connections and processes Redis commands.
 * 
 * Responsibilities:
 * - Parse incoming protocol messages
 * - Execute commands against KV store
 * - Serialize and send responses
 * - Handle protocol errors gracefully
 */
class CommandHandler {
public:
    CommandHandler();
    ~CommandHandler();
    
    /**
     * Process client connection.
     * This is the main handler function for TCP server.
     * Returns false to close connection, true to keep it open.
     */
    bool handle_client(ClientConnection& client);
    
    /**
     * Get handler statistics.
     */
    struct Stats {
        size_t connections_handled;
        size_t requests_processed;
        size_t errors_encountered;
    };
    
    Stats get_stats() const;

private:
    KVStore& store_;                      // Reference to KV store
    protocol::Parser parser_;             // Protocol parser for this handler
    
    // Statistics
    std::atomic<size_t> connections_handled_{0};
    std::atomic<size_t> requests_processed_{0};
    std::atomic<size_t> errors_encountered_{0};
    
    /**
     * Process a single request and send response.
     * Returns false on connection error.
     */
    bool process_request(ClientConnection& client, 
                        const protocol::MessagePtr& request);
    
    /**
     * Send response message to client.
     * Handles serialization and error checking.
     */
    bool send_response(ClientConnection& client, 
                      const protocol::MessagePtr& response);
};

/**
 * Factory function for creating command handler.
 * Used with TCP server's ClientHandler callback.
 */
std::function<bool(ClientConnection&)> make_command_handler();

} // namespace scuffedredis

#endif // SCUFFEDREDIS_COMMAND_HANDLER_HPP
