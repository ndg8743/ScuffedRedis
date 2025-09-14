#include "hashtable.hpp"
#include <algorithm>
#include <cstring>

namespace scuffedredis {

// ============================================================================
// MurmurHash3 Implementation (32-bit)
// ============================================================================

static uint32_t murmur3_32(const void* key, size_t len, uint32_t seed) {
    const uint8_t* data = static_cast<const uint8_t*>(key);
    const int nblocks = static_cast<int>(len / 4);
    
    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    
    // Process 4-byte blocks
    const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data + nblocks * 4);
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];
        
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64;
    }
    
    // Process remaining bytes
    const uint8_t* tail = data + nblocks * 4;
    uint32_t k1 = 0;
    
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16; [[fallthrough]];
        case 2: k1 ^= tail[1] << 8; [[fallthrough]];
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
    }
    
    // Finalization
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return h1;
}

// ============================================================================
// HashTable Implementation
// ============================================================================

HashTable::HashTable(size_t initial_capacity) : size_(0) {
    // Round up to power of 2 for better distribution
    size_t capacity = MIN_CAPACITY;
    while (capacity < initial_capacity) {
        capacity *= 2;
    }
    buckets_.resize(capacity);
}

HashTable::~HashTable() = default;

HashTable::HashTable(HashTable&& other) noexcept
    : buckets_(std::move(other.buckets_)), size_(other.size_) {
    other.size_ = 0;
}

HashTable& HashTable::operator=(HashTable&& other) noexcept {
    if (this != &other) {
        buckets_ = std::move(other.buckets_);
        size_ = other.size_;
        other.size_ = 0;
    }
    return *this;
}

size_t HashTable::hash(const std::string& key) const {
    // Use MurmurHash3 with a fixed seed
    uint32_t hash_val = murmur3_32(key.data(), key.size(), 0x12345678);
    return hash_val % buckets_.size();
}

std::pair<HashTable::Node*, HashTable::Node*> 
HashTable::find_in_bucket(size_t bucket, const std::string& key) const {
    Node* prev = nullptr;
    Node* curr = buckets_[bucket].get();
    
    while (curr) {
        if (curr->key == key) {
            return {curr, prev};
        }
        prev = curr;
        curr = curr->next.get();
    }
    
    return {nullptr, nullptr};
}

bool HashTable::set(const std::string& key, const std::string& value) {
    // Check if resize is needed before insertion
    if (load_factor() > MAX_LOAD_FACTOR) {
        resize();
    }
    
    size_t bucket = hash(key);
    auto [node, prev] = find_in_bucket(bucket, key);
    
    if (node) {
        // Key exists, update value
        node->value = value;
        return false;  // Not a new insertion
    }
    
    // Insert new node at head of bucket
    auto new_node = std::make_unique<Node>(key, value);
    new_node->next = std::move(buckets_[bucket]);
    buckets_[bucket] = std::move(new_node);
    size_++;
    
    return true;  // New insertion
}

std::optional<std::string> HashTable::get(const std::string& key) const {
    size_t bucket = hash(key);
    auto [node, prev] = find_in_bucket(bucket, key);
    
    if (node) {
        return node->value;
    }
    
    return std::nullopt;
}

bool HashTable::del(const std::string& key) {
    size_t bucket = hash(key);
    auto [node, prev] = find_in_bucket(bucket, key);
    
    if (!node) {
        return false;  // Key not found
    }
    
    if (prev) {
        // Node is in middle or end of chain
        prev->next = std::move(node->next);
    } else {
        // Node is at head of bucket
        buckets_[bucket] = std::move(node->next);
    }
    
    size_--;
    return true;
}

bool HashTable::exists(const std::string& key) const {
    size_t bucket = hash(key);
    auto [node, prev] = find_in_bucket(bucket, key);
    return node != nullptr;
}

void HashTable::clear() {
    for (auto& bucket : buckets_) {
        bucket.reset();
    }
    size_ = 0;
}

void HashTable::resize() {
    size_t new_capacity = buckets_.size() * 2;
    std::vector<std::unique_ptr<Node>> new_buckets(new_capacity);
    
    // Rehash all existing entries
    for (auto& bucket : buckets_) {
        Node* curr = bucket.get();
        while (curr) {
            // Calculate new bucket index
            size_t new_bucket = murmur3_32(curr->key.data(), 
                                          curr->key.size(), 
                                          0x12345678) % new_capacity;
            
            // Move node to new bucket
            Node* next = curr->next.release();
            curr->next = std::move(new_buckets[new_bucket]);
            new_buckets[new_bucket].reset(curr);
            
            curr = next;
        }
        bucket.release();  // Already moved, just release ownership
    }
    
    buckets_ = std::move(new_buckets);
}

bool HashTable::matches_pattern(const std::string& str, 
                               const std::string& pattern) const {
    // Simple wildcard matching supporting * only
    if (pattern == "*") {
        return true;
    }
    
    size_t str_idx = 0;
    size_t pat_idx = 0;
    size_t star_idx = std::string::npos;
    size_t match_idx = 0;
    
    while (str_idx < str.size()) {
        if (pat_idx < pattern.size() && 
            (pattern[pat_idx] == str[str_idx] || pattern[pat_idx] == '?')) {
            // Characters match or '?' wildcard
            str_idx++;
            pat_idx++;
        } else if (pat_idx < pattern.size() && pattern[pat_idx] == '*') {
            // '*' wildcard - remember position
            star_idx = pat_idx++;
            match_idx = str_idx;
        } else if (star_idx != std::string::npos) {
            // Backtrack to last '*'
            pat_idx = star_idx + 1;
            str_idx = ++match_idx;
        } else {
            // No match
            return false;
        }
    }
    
    // Check remaining pattern characters
    while (pat_idx < pattern.size() && pattern[pat_idx] == '*') {
        pat_idx++;
    }
    
    return pat_idx == pattern.size();
}

std::vector<std::string> HashTable::keys(const std::string& pattern) const {
    std::vector<std::string> result;
    
    // Iterate through all buckets
    for (const auto& bucket : buckets_) {
        Node* curr = bucket.get();
        while (curr) {
            if (matches_pattern(curr->key, pattern)) {
                result.push_back(curr->key);
            }
            curr = curr->next.get();
        }
    }
    
    return result;
}

HashTable::Stats HashTable::get_stats() const {
    Stats stats{};
    stats.total_entries = size_;
    stats.total_buckets = buckets_.size();
    stats.load_factor = load_factor();
    
    size_t total_chain_length = 0;
    
    for (const auto& bucket : buckets_) {
        if (bucket) {
            stats.used_buckets++;
            
            size_t chain_length = 0;
            Node* curr = bucket.get();
            while (curr) {
                chain_length++;
                curr = curr->next.get();
            }
            
            total_chain_length += chain_length;
            stats.max_chain_length = std::max(stats.max_chain_length, chain_length);
        }
    }
    
    if (stats.used_buckets > 0) {
        stats.average_chain_length = static_cast<double>(total_chain_length) / 
                                    stats.used_buckets;
    }
    
    return stats;
}

// ============================================================================
// Iterator Implementation
// ============================================================================

HashTable::Iterator& HashTable::Iterator::operator++() {
    if (!node_ || !table_) {
        return *this;
    }
    
    // Move to next node in current bucket
    node_ = node_->next.get();
    
    // If no more nodes in current bucket, find next non-empty bucket
    if (!node_) {
        advance_to_next();
    }
    
    return *this;
}

void HashTable::Iterator::advance_to_next() {
    bucket_++;
    while (bucket_ < table_->buckets_.size()) {
        if (table_->buckets_[bucket_]) {
            node_ = table_->buckets_[bucket_].get();
            return;
        }
        bucket_++;
    }
    
    // Reached end
    node_ = nullptr;
    table_ = nullptr;
}

bool HashTable::Iterator::operator==(const Iterator& other) const {
    return table_ == other.table_ && 
           bucket_ == other.bucket_ && 
           node_ == other.node_;
}

std::pair<const std::string&, std::string&> HashTable::Iterator::operator*() {
    return {node_->key, node_->value};
}

// ============================================================================
// ConcurrentHashTable Implementation
// ============================================================================

ConcurrentHashTable::ConcurrentHashTable(size_t initial_capacity)
    : table_(initial_capacity) {
}

bool ConcurrentHashTable::set(const std::string& key, const std::string& value) {
    std::unique_lock lock(mutex_);
    return table_.set(key, value);
}

std::optional<std::string> ConcurrentHashTable::get(const std::string& key) const {
    std::shared_lock lock(mutex_);
    return table_.get(key);
}

bool ConcurrentHashTable::del(const std::string& key) {
    std::unique_lock lock(mutex_);
    return table_.del(key);
}

bool ConcurrentHashTable::exists(const std::string& key) const {
    std::shared_lock lock(mutex_);
    return table_.exists(key);
}

std::vector<std::string> ConcurrentHashTable::keys(const std::string& pattern) const {
    std::shared_lock lock(mutex_);
    return table_.keys(pattern);
}

void ConcurrentHashTable::clear() {
    std::unique_lock lock(mutex_);
    table_.clear();
}

size_t ConcurrentHashTable::size() const {
    std::shared_lock lock(mutex_);
    return table_.size();
}

} // namespace scuffedredis
