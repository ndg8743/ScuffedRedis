#include "kv_store.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace scuffedredis {

KVStore::KVStore() {
    init_handlers();
    LOG_INFO("Key-Value store initialized");
}

KVStore::~KVStore() {
    LOG_INFO(format_log("KV store shutting down. Processed ", 
                       commands_processed_.load(), " commands"));
}

void KVStore::init_handlers() {
    // Register all command handlers
    // Using lambdas to bind this pointer
    
    handlers_["GET"] = [this](const auto& args) { 
        return handle_get(args); 
    };
    
    handlers_["SET"] = [this](const auto& args) { 
        return handle_set(args); 
    };
    
    handlers_["DEL"] = [this](const auto& args) { 
        return handle_del(args); 
    };
    
    handlers_["EXISTS"] = [this](const auto& args) { 
        return handle_exists(args); 
    };
    
    handlers_["KEYS"] = [this](const auto& args) { 
        return handle_keys(args); 
    };
    
    handlers_["PING"] = [this](const auto& args) { 
        return handle_ping(args); 
    };
    
    handlers_["ECHO"] = [this](const auto& args) { 
        return handle_echo(args); 
    };
    
    handlers_["FLUSHDB"] = [this](const auto& args) { 
        return handle_flushdb(args); 
    };
    
    handlers_["DBSIZE"] = [this](const auto& args) { 
        return handle_dbsize(args); 
    };
    
    handlers_["INFO"] = [this](const auto& args) { 
        return handle_info(args); 
    };
}

std::string KVStore::to_upper(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char c) { return std::toupper(c); });
    return result;
}

protocol::MessagePtr KVStore::execute_command(const protocol::MessagePtr& request) {
    commands_processed_++;
    
    // Parse command from request
    auto args = protocol::utils::parse_command(request);
    
    if (args.empty()) {
        return protocol::utils::error_response("ERR invalid command format");
    }
    
    return execute_raw(args);
}

protocol::MessagePtr KVStore::execute_raw(const std::vector<std::string>& args) {
    if (args.empty()) {
        return protocol::utils::error_response("ERR empty command");
    }
    
    // Get command name (case-insensitive)
    std::string cmd = to_upper(args[0]);
    
    // Find handler
    auto it = handlers_.find(cmd);
    if (it == handlers_.end()) {
        return protocol::utils::error_response("ERR unknown command '" + args[0] + "'");
    }
    
    // Execute handler
    try {
        return it->second(args);
    } catch (const std::exception& e) {
        LOG_ERROR(format_log("Command execution error: ", e.what()));
        return protocol::utils::error_response("ERR " + std::string(e.what()));
    }
}

// ============================================================================
// Command Handlers
// ============================================================================

protocol::MessagePtr KVStore::handle_get(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'GET'");
    }
    
    get_commands_++;
    
    const std::string& key = args[1];
    auto value = store_.get(key);
    
    if (value.has_value()) {
        // Return bulk string with value
        return protocol::Message::make_bulk_string(value.value());
    } else {
        // Key doesn't exist - return nil
        return protocol::utils::nil_response();
    }
}

protocol::MessagePtr KVStore::handle_set(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'SET'");
    }
    
    set_commands_++;
    
    const std::string& key = args[1];
    const std::string& value = args[2];
    
    // TODO: Handle additional SET options (EX, PX, NX, XX) later
    
    store_.set(key, value);
    return protocol::utils::ok_response();
}

protocol::MessagePtr KVStore::handle_del(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'DEL'");
    }
    
    del_commands_++;
    
    int64_t deleted = 0;
    
    // Delete each key
    for (size_t i = 1; i < args.size(); i++) {
        if (store_.del(args[i])) {
            deleted++;
        }
    }
    
    // Return number of keys deleted
    return protocol::Message::make_integer(deleted);
}

protocol::MessagePtr KVStore::handle_exists(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'EXISTS'");
    }
    
    int64_t count = 0;
    
    // Check each key
    for (size_t i = 1; i < args.size(); i++) {
        if (store_.exists(args[i])) {
            count++;
        }
    }
    
    // Return number of keys that exist
    return protocol::Message::make_integer(count);
}

protocol::MessagePtr KVStore::handle_keys(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'KEYS'");
    }
    
    const std::string& pattern = args[1];
    auto keys = store_.keys(pattern);
    
    // Convert to array of bulk strings
    protocol::MessageArray array;
    array.reserve(keys.size());
    
    for (const auto& key : keys) {
        array.push_back(protocol::Message::make_bulk_string(key));
    }
    
    return protocol::Message::make_array(array);
}

protocol::MessagePtr KVStore::handle_ping(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        // No argument - return PONG
        return protocol::utils::pong_response();
    } else if (args.size() == 2) {
        // Echo back the argument
        return protocol::Message::make_bulk_string(args[1]);
    } else {
        return protocol::utils::error_response("ERR wrong number of arguments for 'PING'");
    }
}

protocol::MessagePtr KVStore::handle_echo(const std::vector<std::string>& args) {
    if (args.size() != 2) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'ECHO'");
    }
    
    // Echo back the message
    return protocol::Message::make_bulk_string(args[1]);
}

protocol::MessagePtr KVStore::handle_flushdb(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'FLUSHDB'");
    }
    
    store_.clear();
    LOG_INFO("Database flushed");
    
    return protocol::utils::ok_response();
}

protocol::MessagePtr KVStore::handle_dbsize(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        return protocol::utils::error_response("ERR wrong number of arguments for 'DBSIZE'");
    }
    
    // Return number of keys in database
    return protocol::Message::make_integer(static_cast<int64_t>(store_.size()));
}

protocol::MessagePtr KVStore::handle_info(const std::vector<std::string>& args) {
    // Build info string
    std::ostringstream info;
    
    info << "# Server\r\n";
    info << "redis_version:ScuffedRedis-0.1.0\r\n";
    info << "redis_mode:standalone\r\n";
    info << "process_id:" << 1234 << "\r\n";  // Placeholder
    info << "\r\n";
    
    info << "# Clients\r\n";
    info << "connected_clients:1\r\n";  // Placeholder
    info << "\r\n";
    
    info << "# Memory\r\n";
    info << "used_memory:" << (store_.size() * 100) << "\r\n";  // Rough estimate
    info << "\r\n";
    
    info << "# Stats\r\n";
    info << "total_commands_processed:" << commands_processed_.load() << "\r\n";
    info << "instantaneous_ops_per_sec:0\r\n";  // Placeholder
    info << "\r\n";
    
    info << "# Keyspace\r\n";
    info << "db0:keys=" << store_.size() << ",expires=0\r\n";
    
    return protocol::Message::make_bulk_string(info.str());
}

void KVStore::clear() {
    store_.clear();
    
    // Reset statistics
    commands_processed_ = 0;
    get_commands_ = 0;
    set_commands_ = 0;
    del_commands_ = 0;
}

KVStore::Stats KVStore::get_stats() const {
    Stats stats;
    stats.keys_count = store_.size();
    stats.memory_usage = store_.size() * 100;  // Rough estimate
    stats.commands_processed = commands_processed_.load();
    stats.get_commands = get_commands_.load();
    stats.set_commands = set_commands_.load();
    stats.del_commands = del_commands_.load();
    
    return stats;
}

} // namespace scuffedredis
