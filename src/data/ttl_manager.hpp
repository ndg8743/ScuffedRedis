#ifndef SCUFFEDREDIS_TTL_MANAGER_HPP
#define SCUFFEDREDIS_TTL_MANAGER_HPP

/**
 * TTL (Time To Live) Manager for ScuffedRedis.
 * 
 * Handles key expiration and automatic cleanup.
 * Uses a heap-based timer for efficient expiration checking.
 */

#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <functional>
#include <thread>

namespace scuffedredis {

/**
 * TTL entry for tracking key expiration.
 */
struct TTLEntry {
    std::string key;                                    // Key to expire
    std::chrono::steady_clock::time_point expire_time; // When to expire
    uint64_t id;                                       // Unique ID for heap management
    
    TTLEntry(const std::string& k, 
             std::chrono::steady_clock::time_point exp,
             uint64_t i) 
        : key(k), expire_time(exp), id(i) {}
    
    // For min-heap ordering (earliest expiration first)
    bool operator>(const TTLEntry& other) const {
        return expire_time > other.expire_time;
    }
};

/**
 * TTL Manager for handling key expiration.
 * 
 * Features:
 * - Min-heap for efficient expiration checking
 * - Thread-safe operations
 * - Automatic cleanup of expired keys
 * - Support for EX and PX expiration modes
 */
class TTLManager {
public:
    using ExpirationCallback = std::function<void(const std::string&)>;
    
    TTLManager();
    ~TTLManager();
    
    /**
     * Set TTL for a key.
     * key: Key to set expiration for
     * seconds: Expiration time in seconds (0 to remove TTL)
     * Returns true if TTL was set successfully.
     */
    bool set_ttl(const std::string& key, int64_t seconds);
    
    /**
     * Set TTL for a key in milliseconds.
     * key: Key to set expiration for
     * milliseconds: Expiration time in milliseconds (0 to remove TTL)
     * Returns true if TTL was set successfully.
     */
    bool set_ttl_ms(const std::string& key, int64_t milliseconds);
    
    /**
     * Get TTL for a key in seconds.
     * Returns -1 if key has no TTL, -2 if key doesn't exist.
     */
    int64_t get_ttl(const std::string& key) const;
    
    /**
     * Get TTL for a key in milliseconds.
     * Returns -1 if key has no TTL, -2 if key doesn't exist.
     */
    int64_t get_ttl_ms(const std::string& key) const;
    
    /**
     * Check if key has TTL set.
     */
    bool has_ttl(const std::string& key) const;
    
    /**
     * Remove TTL for a key.
     * Returns true if TTL was removed.
     */
    bool remove_ttl(const std::string& key);
    
    /**
     * Check and expire keys.
     * Should be called periodically to clean up expired keys.
     * Returns number of keys expired.
     */
    size_t check_expirations();
    
    /**
     * Set callback for key expiration.
     * Called when a key expires.
     */
    void set_expiration_callback(ExpirationCallback callback) {
        expiration_callback_ = callback;
    }
    
    /**
     * Get number of keys with TTL.
     */
    size_t size() const;
    
    /**
     * Clear all TTL entries.
     */
    void clear();
    
    /**
     * Get TTL statistics.
     */
    struct Stats {
        size_t total_ttl_entries;
        size_t expired_this_check;
        size_t total_expired;
        double average_ttl_seconds;
    };
    
    Stats get_stats() const;

private:
    // Min-heap for efficient expiration checking
    std::vector<TTLEntry> expiration_heap_;
    
    // Map from key to heap index for O(1) updates
    std::unordered_map<std::string, size_t> key_to_index_;
    
    // Map from key to TTL entry ID for tracking
    std::unordered_map<std::string, uint64_t> key_to_id_;
    
    // Next ID for TTL entries
    std::atomic<uint64_t> next_id_;
    
    // Statistics
    mutable std::atomic<size_t> total_expired_;
    
    // Callback for key expiration
    ExpirationCallback expiration_callback_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    /**
     * Heap operations for maintaining min-heap property.
     */
    void heapify_up(size_t index);
    void heapify_down(size_t index);
    void swap_entries(size_t i, size_t j);
    
    /**
     * Remove entry from heap by index.
     */
    void remove_from_heap(size_t index);
    
    /**
     * Get current time.
     */
    std::chrono::steady_clock::time_point now() const {
        return std::chrono::steady_clock::now();
    }
    
    /**
     * Calculate expiration time from seconds.
     */
    std::chrono::steady_clock::time_point calculate_expire_time(int64_t seconds) const {
        return now() + std::chrono::seconds(seconds);
    }
    
    /**
     * Calculate expiration time from milliseconds.
     */
    std::chrono::steady_clock::time_point calculate_expire_time_ms(int64_t milliseconds) const {
        return now() + std::chrono::milliseconds(milliseconds);
    }
};

/**
 * Global TTL manager instance.
 */
class TTLManagerSingleton {
public:
    static TTLManagerSingleton& instance() {
        static TTLManagerSingleton singleton;
        return singleton;
    }
    
    TTLManager& get_manager() { return manager_; }
    
    /**
     * Start background expiration checking.
     */
    void start_background_checker();
    
    /**
     * Stop background expiration checking.
     */
    void stop_background_checker();

private:
    TTLManagerSingleton() = default;
    TTLManager manager_;
    std::thread background_thread_;
    std::atomic<bool> running_{false};
    
    void background_checker_loop();
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_TTL_MANAGER_HPP
