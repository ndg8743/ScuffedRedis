#include "redis_client.hpp"
#include "utils/logger.hpp"
#include <sstream>
#include <iostream>

namespace scuffedredis {

RedisClient::RedisClient() {
    // Configure logging for client
    Logger::instance().set_level(LogLevel::ERROR_LEVEL);  // Only show errors
}

RedisClient::~RedisClient() {
    disconnect();
}

bool RedisClient::connect(const std::string& host, uint16_t port, int timeout_ms) {
    return client_.connect(host, port, timeout_ms);
}

void RedisClient::disconnect() {
    client_.disconnect();
}

protocol::MessagePtr RedisClient::send_command(const protocol::MessagePtr& cmd) {
    if (!is_connected()) {
        return protocol::utils::error_response("Not connected to server");
    }
    
    // Serialize command
    auto data = cmd->serialize();
    
    // Send to server
    if (!client_.send_raw(data.data(), data.size())) {
        return protocol::utils::error_response("Failed to send command");
    }
    
    // Read response
    uint8_t buffer[4096];
    ssize_t received = client_.receive_with_timeout(buffer, sizeof(buffer), 5000);
    
    if (received <= 0) {
        return protocol::utils::error_response("Failed to receive response");
    }
    
    // Parse response
    parser_.feed(buffer, received);
    
    // Keep reading if we don't have a complete message
    while (!parser_.has_message() && is_connected()) {
        received = client_.receive_with_timeout(buffer, sizeof(buffer), 100);
        if (received > 0) {
            parser_.feed(buffer, received);
        } else {
            break;
        }
    }
    
    // Get parsed message
    auto response = parser_.parse_message();
    if (!response) {
        return protocol::utils::error_response("Failed to parse response");
    }
    
    return response;
}

protocol::MessagePtr RedisClient::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        return protocol::utils::error_response("Empty command");
    }
    
    // Create command message
    auto cmd = protocol::utils::make_command(args);
    
    // Send and receive
    return send_command(cmd);
}

std::optional<std::string> RedisClient::execute_string(const std::vector<std::string>& args) {
    auto response = execute(args);
    
    if (!response) {
        return std::nullopt;
    }
    
    if (response->is_string()) {
        return response->as_string();
    }
    
    return std::nullopt;
}

std::vector<std::string> RedisClient::parse_command_line(const std::string& line) {
    std::vector<std::string> args;
    std::string current;
    bool in_quotes = false;
    bool escape_next = false;
    
    for (char c : line) {
        if (escape_next) {
            // Handle escape sequences
            switch (c) {
                case 'n': current += '\n'; break;
                case 'r': current += '\r'; break;
                case 't': current += '\t'; break;
                case '\\': current += '\\'; break;
                case '"': current += '"'; break;
                default: current += c; break;
            }
            escape_next = false;
        } else if (c == '\\') {
            escape_next = true;
        } else if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ' ' && !in_quotes) {
            // Space outside quotes - end of argument
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    // Add last argument
    if (!current.empty()) {
        args.push_back(current);
    }
    
    return args;
}

std::string RedisClient::format_response(const protocol::MessagePtr& response) {
    if (!response) {
        return "(nil)";
    }
    
    std::ostringstream oss;
    
    switch (response->get_type()) {
        case protocol::MessageType::SIMPLE_STRING:
            oss << response->as_string();
            break;
            
        case protocol::MessageType::ERROR_MSG:
            oss << "(error) " << response->as_string();
            break;
            
        case protocol::MessageType::INTEGER:
            oss << "(integer) " << response->as_integer();
            break;
            
        case protocol::MessageType::BULK_STRING:
            oss << "\"" << response->as_string() << "\"";
            break;
            
        case protocol::MessageType::ARRAY: {
            auto arr = response->as_array();
            if (!arr || arr->empty()) {
                oss << "(empty array)";
            } else {
                // Format array elements
                for (size_t i = 0; i < arr->size(); i++) {
                    oss << (i + 1) << ") ";
                    if ((*arr)[i]) {
                        oss << format_response((*arr)[i]);
                    } else {
                        oss << "(nil)";
                    }
                    if (i < arr->size() - 1) {
                        oss << "\n";
                    }
                }
            }
            break;
        }
            
        case protocol::MessageType::NULL_VALUE:
            oss << "(nil)";
            break;
    }
    
    return oss.str();
}

// Convenience methods
bool RedisClient::set(const std::string& key, const std::string& value) {
    auto response = execute({"SET", key, value});
    return response && response->is_string() && response->as_string() == "OK";
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    auto response = execute({"GET", key});
    
    if (!response || response->is_null()) {
        return std::nullopt;
    }
    
    if (response->is_string()) {
        return response->as_string();
    }
    
    return std::nullopt;
}

bool RedisClient::del(const std::string& key) {
    auto response = execute({"DEL", key});
    return response && response->is_integer() && response->as_integer() > 0;
}

bool RedisClient::exists(const std::string& key) {
    auto response = execute({"EXISTS", key});
    return response && response->is_integer() && response->as_integer() > 0;
}

std::vector<std::string> RedisClient::keys(const std::string& pattern) {
    std::vector<std::string> result;
    
    auto response = execute({"KEYS", pattern});
    
    if (response && response->is_array()) {
        auto arr = response->as_array();
        if (arr) {
            for (const auto& elem : *arr) {
                if (elem && elem->is_string()) {
                    result.push_back(elem->as_string());
                }
            }
        }
    }
    
    return result;
}

bool RedisClient::ping() {
    auto response = execute({"PING"});
    return response && response->is_string() && response->as_string() == "PONG";
}

} // namespace scuffedredis
