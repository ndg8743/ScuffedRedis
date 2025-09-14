#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>

namespace scuffedredis {
namespace data {

// Simple hashtable for key-value storage
// Need this because we need fast O(1) average lookup/insert/delete operations
template<typename K, typename V>
class HashTable {
public:
    struct Node {
        K key;
        V value;
        std::unique_ptr<Node> next;
        
        Node(const K& k, const V& v) : key(k), value(v) {}
    };
    
    HashTable(size_t initialCapacity = 16);
    ~HashTable() = default;
    
    // Basic operations
    bool insert(const K& key, const V& value);
    bool get(const K& key, V& value) const;
    bool remove(const K& key);
    bool exists(const K& key) const;
    
    // Utility operations
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void clear();
    
    // Get all keys (for KEYS command)
    std::vector<K> getAllKeys() const;
    
    // Thread-safe operations
    std::mutex& getMutex() const { return mutex_; }

private:
    std::vector<std::unique_ptr<Node>> buckets_;
    size_t size_;
    size_t capacity_;
    mutable std::mutex mutex_;
    
    // Hash function
    size_t hash(const K& key) const;
    
    // Resize table when load factor gets too high
    void resize();
    
    // Get load factor
    double getLoadFactor() const { return static_cast<double>(size_) / capacity_; }
};

} // namespace data
} // namespace scuffedredis
