#include "command_handler.hpp"
#include "utils/logger.hpp"
#include <iostream>

namespace scuffedredis {

CommandHandler::CommandHandler() 
    : store_(KVStoreManager::instance().get_store()) {
    LOG_INFO("Command handler initialized");
}

CommandHandler::~CommandHandler() {
    LOG_INFO(format_log("Command handler shutting down. Handled ", 
                       connections_handled_.load(), " connections"));
}

bool CommandHandler::handle_client(ClientConnection& client) {
    connections_handled_++;
    
    // Get data from client's read buffer
    const auto& buffer = client.get_read_buffer();
    
    if (buffer.empty()) {
        return true;  // Keep connection open, no data yet
    }
    
    // Feed data to protocol parser
    parser_.feed(buffer);
    
    // Process all complete messages
    bool connection_ok = true;
    size_t messages_processed = 0;
    
    while (parser_.has_message()) {
        auto request = parser_.parse_message();
        
        if (!request) {
            // Parse error - shouldn't happen if has_message() returned true
            LOG_ERROR("Protocol parse error despite has_message() returning true");
            errors_encountered_++;
            break;
        }
        
        // Process request and send response
        if (!process_request(client, request)) {
            connection_ok = false;
            break;
        }
        
        messages_processed++;
    }
    
    // Consume processed bytes from client buffer
    // For simplicity, clear entire buffer after processing
    // In production, would track exact bytes consumed
    if (messages_processed > 0) {
        client.consume_bytes(buffer.size());
    }
    
    return connection_ok;
}

bool CommandHandler::process_request(ClientConnection& client, 
                                    const protocol::MessagePtr& request) {
    requests_processed_++;
    
    // Log the request for debugging
    LOG_DEBUG(format_log("Processing request from ", client.get_client_info()));
    
    // Execute command against KV store
    protocol::MessagePtr response;
    
    try {
        response = store_.execute_command(request);
    } catch (const std::exception& e) {
        LOG_ERROR(format_log("Command execution error: ", e.what()));
        response = protocol::utils::error_response("ERR internal error");
        errors_encountered_++;
    }
    
    // Send response back to client
    return send_response(client, response);
}

bool CommandHandler::send_response(ClientConnection& client, 
                                  const protocol::MessagePtr& response) {
    if (!response) {
        LOG_ERROR("Attempted to send null response");
        return false;
    }
    
    // Serialize response
    auto data = response->serialize();
    
    // Send to client
    bool success = client.write(data.data(), data.size());
    
    if (!success) {
        LOG_ERROR(format_log("Failed to send response to ", client.get_client_info()));
        errors_encountered_++;
    }
    
    return success;
}

CommandHandler::Stats CommandHandler::get_stats() const {
    Stats stats;
    stats.connections_handled = connections_handled_.load();
    stats.requests_processed = requests_processed_.load();
    stats.errors_encountered = errors_encountered_.load();
    return stats;
}

// Factory function for creating command handler
std::function<bool(ClientConnection&)> make_command_handler() {
    // Create a shared command handler
    // Each connection will use the same handler instance
    static auto handler = std::make_shared<CommandHandler>();
    
    return [handler](ClientConnection& client) -> bool {
        return handler->handle_client(client);
    };
}

} // namespace scuffedredis
