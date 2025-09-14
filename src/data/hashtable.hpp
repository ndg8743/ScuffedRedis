#ifndef SCUFFEDREDIS_HASHTABLE_HPP
#define SCUFFEDREDIS_HASHTABLE_HPP

/**
 * Hash table implementation for ScuffedRedis.
 * 
 * Uses separate chaining for collision resolution.
 * Supports dynamic resizing when load factor exceeds threshold.
 * Thread-safe version will be added later.
 */

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <shared_mutex>

namespace scuffedredis {

/**
 * Hash table with string keys and values.
 * 
 * Features:
 * - Separate chaining with linked lists
 * - Dynamic resizing at 75% load factor
 * - MurmurHash3 for better distribution
 * - Cache-friendly memory layout
 */
class HashTable {
public:
    // Node for separate chaining
    struct Node {
        std::string key;
        std::string value;
        std::unique_ptr<Node> next;
        
        Node(const std::string& k, const std::string& v) 
            : key(k), value(v), next(nullptr) {}
    };
    
    // Iterator for traversing the hash table
    class Iterator {
    public:
        Iterator(HashTable* table, size_t bucket, Node* node)
            : table_(table), bucket_(bucket), node_(node) {
            // Find first non-empty bucket if current is null
            if (!node_ && table_) {
                advance_to_next();
            }
        }
        
        // Iterator operations
        Iterator& operator++();
        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const { return !(*this == other); }
        std::pair<const std::string&, std::string&> operator*();
        
    private:
        HashTable* table_;
        size_t bucket_;
        Node* node_;
        
        void advance_to_next();
    };
    
    // Constructor with initial capacity
    explicit HashTable(size_t initial_capacity = 16);
    ~HashTable();
    
    // Disable copy, allow move
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&& other) noexcept;
    HashTable& operator=(HashTable&& other) noexcept;
    
    /**
     * Insert or update a key-value pair.
     * Returns true if key was newly inserted, false if updated.
     */
    bool set(const std::string& key, const std::string& value);
    
    /**
     * Get value for a key.
     * Returns nullopt if key doesn't exist.
     */
    std::optional<std::string> get(const std::string& key) const;
    
    /**
     * Delete a key-value pair.
     * Returns true if key existed and was deleted.
     */
    bool del(const std::string& key);
    
    /**
     * Check if key exists.
     */
    bool exists(const std::string& key) const;
    
    /**
     * Get all keys matching a pattern.
     * Pattern supports * wildcard (e.g., "user:*").
     * Simple implementation for now.
     */
    std::vector<std::string> keys(const std::string& pattern = "*") const;
    
    /**
     * Clear all entries.
     */
    void clear();
    
    /**
     * Get number of entries.
     */
    size_t size() const { return size_; }
    
    /**
     * Check if empty.
     */
    bool empty() const { return size_ == 0; }
    
    /**
     * Get current capacity (number of buckets).
     */
    size_t capacity() const { return buckets_.size(); }
    
    /**
     * Get load factor (size / capacity).
     */
    double load_factor() const {
        return buckets_.empty() ? 0.0 : 
               static_cast<double>(size_) / buckets_.size();
    }
    
    // Iterator support
    Iterator begin() { return Iterator(this, 0, nullptr); }
    Iterator end() { return Iterator(nullptr, 0, nullptr); }
    
    // Statistics for monitoring
    struct Stats {
        size_t total_entries;
        size_t total_buckets;
        size_t used_buckets;
        size_t max_chain_length;
        double average_chain_length;
        double load_factor;
    };
    
    Stats get_stats() const;

private:
    std::vector<std::unique_ptr<Node>> buckets_;  // Array of bucket heads
    size_t size_;                                  // Number of entries
    
    // Configuration
    static constexpr double MAX_LOAD_FACTOR = 0.75;
    static constexpr size_t MIN_CAPACITY = 16;
    
    /**
     * Hash function using MurmurHash3.
     * Better distribution than simple modulo.
     */
    size_t hash(const std::string& key) const;
    
    /**
     * Resize the hash table when load factor exceeds threshold.
     * Rehashes all existing entries.
     */
    void resize();
    
    /**
     * Find node for a given key in a bucket.
     * Returns pair of (node, previous_node) for deletion.
     */
    std::pair<Node*, Node*> find_in_bucket(size_t bucket, 
                                          const std::string& key) const;
    
    /**
     * Check if string matches pattern with wildcards.
     * Simple implementation supporting only * wildcard.
     */
    bool matches_pattern(const std::string& str, 
                        const std::string& pattern) const;
};

/**
 * Thread-safe wrapper for HashTable.
 * Uses read-write lock for better concurrency.
 */
class ConcurrentHashTable {
public:
    explicit ConcurrentHashTable(size_t initial_capacity = 16);
    
    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);
    bool exists(const std::string& key) const;
    std::vector<std::string> keys(const std::string& pattern = "*") const;
    void clear();
    size_t size() const;
    
private:
    HashTable table_;
    mutable std::shared_mutex mutex_;  // Reader-writer lock
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_HASHTABLE_HPP
