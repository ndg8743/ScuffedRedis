#include "hashtable.hpp"
#include "../utils/logger.hpp"
#include <algorithm>

namespace scuffedredis {
namespace data {

template<typename K, typename V>
HashTable<K, V>::HashTable(size_t initialCapacity) 
    : capacity_(initialCapacity), size_(0) {
    buckets_.resize(capacity_);
}

template<typename K, typename V>
bool HashTable<K, V>::insert(const K& key, const V& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t index = hash(key) % capacity_;
    
    // Check if key already exists
    Node* current = buckets_[index].get();
    while (current) {
        if (current->key == key) {
            current->value = value;  // Update existing value
            return true;
        }
        current = current->next.get();
    }
    
    // Insert new node at head of chain
    auto newNode = std::make_unique<Node>(key, value);
    newNode->next = std::move(buckets_[index]);
    buckets_[index] = std::move(newNode);
    size_++;
    
    // Resize if load factor is too high
    if (getLoadFactor() > 0.75) {
        resize();
    }
    
    return true;
}

template<typename K, typename V>
bool HashTable<K, V>::get(const K& key, V& value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t index = hash(key) % capacity_;
    Node* current = buckets_[index].get();
    
    while (current) {
        if (current->key == key) {
            value = current->value;
            return true;
        }
        current = current->next.get();
    }
    
    return false;
}

template<typename K, typename V>
bool HashTable<K, V>::remove(const K& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t index = hash(key) % capacity_;
    Node* current = buckets_[index].get();
    Node* prev = nullptr;
    
    while (current) {
        if (current->key == key) {
            if (prev) {
                prev->next = std::move(current->next);
            } else {
                buckets_[index] = std::move(current->next);
            }
            size_--;
            return true;
        }
        prev = current;
        current = current->next.get();
    }
    
    return false;
}

template<typename K, typename V>
bool HashTable<K, V>::exists(const K& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t index = hash(key) % capacity_;
    Node* current = buckets_[index].get();
    
    while (current) {
        if (current->key == key) {
            return true;
        }
        current = current->next.get();
    }
    
    return false;
}

template<typename K, typename V>
void HashTable<K, V>::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    buckets_.clear();
    buckets_.resize(capacity_);
    size_ = 0;
}

template<typename K, typename V>
std::vector<K> HashTable<K, V>::getAllKeys() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<K> keys;
    keys.reserve(size_);
    
    for (const auto& bucket : buckets_) {
        Node* current = bucket.get();
        while (current) {
            keys.push_back(current->key);
            current = current->next.get();
        }
    }
    
    return keys;
}

template<typename K, typename V>
size_t HashTable<K, V>::hash(const K& key) const {
    // Simple hash function for strings
    if constexpr (std::is_same_v<K, std::string>) {
        std::hash<std::string> hasher;
        return hasher(key);
    } else {
        // For other types, use std::hash
        std::hash<K> hasher;
        return hasher(key);
    }
}

template<typename K, typename V>
void HashTable<K, V>::resize() {
    size_t newCapacity = capacity_ * 2;
    std::vector<std::unique_ptr<Node>> newBuckets(newCapacity);
    
    // Rehash all existing elements
    for (auto& bucket : buckets_) {
        Node* current = bucket.get();
        while (current) {
            size_t newIndex = hash(current->key) % newCapacity;
            auto node = std::make_unique<Node>(current->key, current->value);
            node->next = std::move(newBuckets[newIndex]);
            newBuckets[newIndex] = std::move(node);
            current = current->next.get();
        }
    }
    
    buckets_ = std::move(newBuckets);
    capacity_ = newCapacity;
    
    utils::Logger::debug("Hashtable resized to capacity: {}", capacity_);
}

// Explicit template instantiations for common types
template class HashTable<std::string, std::string>;

} // namespace data
} // namespace scuffedredis
