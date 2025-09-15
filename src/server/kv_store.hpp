#ifndef SCUFFEDREDIS_KV_STORE_HPP
#define SCUFFEDREDIS_KV_STORE_HPP

/**
 * Key-Value Store for ScuffedRedis.
 * 
 * Manages the in-memory database and handles Redis commands.
 * Uses the hashtable for storage and protocol for communication.
 */

#include "data/hashtable.hpp"
#include "data/sorted_set.hpp"
#include "protocol/protocol.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <atomic>

namespace scuffedredis {

/**
 * Command handler function type.
 * Takes command arguments and returns response message.
 */
using CommandHandlerFunc = std::function<protocol::MessagePtr(
    const std::vector<std::string>&)>;

/**
 * Key-Value store with Redis command support.
 * 
 * Supported commands:
 * - GET key
 * - SET key value
 * - DEL key [key ...]
 * - EXISTS key [key ...]
 * - KEYS pattern
 * - PING [message]
 * - ECHO message
 * - FLUSHDB
 * - DBSIZE
 * - INFO
 * - ZADD key score member [score member ...]
 * - ZRANGE key start stop [WITHSCORES]
 * - ZRANK key member
 * - ZREM key member [member ...]
 * - ZSCORE key member
 * - ZCARD key
 */
class KVStore {
public:
    KVStore();
    ~KVStore();
    
    /**
     * Execute a Redis command.
     * Parses the command and dispatches to appropriate handler.
     */
    protocol::MessagePtr execute_command(const protocol::MessagePtr& request);
    
    /**
     * Execute a raw command (for testing).
     * Command format: ["SET", "key", "value"]
     */
    protocol::MessagePtr execute_raw(const std::vector<std::string>& args);
    
    /**
     * Get store statistics.
     */
    struct Stats {
        size_t keys_count;
        size_t memory_usage;  // Approximate
        size_t commands_processed;
        size_t get_commands;
        size_t set_commands;
        size_t del_commands;
    };
    
    Stats get_stats() const;
    
    /**
     * Clear all data.
     */
    void clear();

private:
    ConcurrentHashTable store_;                              // Main data store
    SortedSetManager sorted_sets_;                          // Sorted sets store
    std::unordered_map<std::string, CommandHandlerFunc> handlers_;  // Command handlers
    
    // Statistics counters
    mutable std::atomic<size_t> commands_processed_{0};
    mutable std::atomic<size_t> get_commands_{0};
    mutable std::atomic<size_t> set_commands_{0};
    mutable std::atomic<size_t> del_commands_{0};
    
    /**
     * Initialize command handlers.
     */
    void init_handlers();
    
    // Command implementations
    protocol::MessagePtr handle_get(const std::vector<std::string>& args);
    protocol::MessagePtr handle_set(const std::vector<std::string>& args);
    protocol::MessagePtr handle_del(const std::vector<std::string>& args);
    protocol::MessagePtr handle_exists(const std::vector<std::string>& args);
    protocol::MessagePtr handle_keys(const std::vector<std::string>& args);
    protocol::MessagePtr handle_ping(const std::vector<std::string>& args);
    protocol::MessagePtr handle_echo(const std::vector<std::string>& args);
    protocol::MessagePtr handle_flushdb(const std::vector<std::string>& args);
    protocol::MessagePtr handle_dbsize(const std::vector<std::string>& args);
    protocol::MessagePtr handle_info(const std::vector<std::string>& args);
    
    // Sorted set command handlers
    protocol::MessagePtr handle_zadd(const std::vector<std::string>& args);
    protocol::MessagePtr handle_zrange(const std::vector<std::string>& args);
    protocol::MessagePtr handle_zrank(const std::vector<std::string>& args);
    protocol::MessagePtr handle_zrem(const std::vector<std::string>& args);
    protocol::MessagePtr handle_zscore(const std::vector<std::string>& args);
    protocol::MessagePtr handle_zcard(const std::vector<std::string>& args);
    
    /**
     * Convert command name to uppercase.
     * Redis commands are case-insensitive.
     */
    std::string to_upper(const std::string& str) const;
};

/**
 * Global KV store instance manager.
 * Provides singleton access to the store.
 */
class KVStoreManager {
public:
    static KVStoreManager& instance() {
        static KVStoreManager manager;
        return manager;
    }
    
    KVStore& get_store() { return store_; }
    
private:
    KVStoreManager() = default;
    KVStore store_;
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_KV_STORE_HPP
