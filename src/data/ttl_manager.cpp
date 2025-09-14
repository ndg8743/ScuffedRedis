#include "ttl_manager.hpp"
#include "utils/logger.hpp"
#include <algorithm>
#include <chrono>

namespace scuffedredis {

// ============================================================================
// TTLManager Implementation
// ============================================================================

TTLManager::TTLManager() : next_id_(1), total_expired_(0) {
}

TTLManager::~TTLManager() {
    clear();
}

bool TTLManager::set_ttl(const std::string& key, int64_t seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (seconds <= 0) {
        // Remove TTL
        return remove_ttl(key);
    }
    
    auto expire_time = calculate_expire_time(seconds);
    uint64_t id = next_id_++;
    
    // Check if key already has TTL
    auto it = key_to_index_.find(key);
    if (it != key_to_index_.end()) {
        // Update existing TTL
        size_t index = it->second;
        expiration_heap_[index].expire_time = expire_time;
        expiration_heap_[index].id = id;
        
        // Update ID mapping
        key_to_id_[key] = id;
        
        // Restore heap property
        heapify_up(index);
        heapify_down(index);
        
        LOG_DEBUG(format_log("Updated TTL for key '", key, "' to ", seconds, " seconds"));
        return true;
    }
    
    // Add new TTL entry
    expiration_heap_.emplace_back(key, expire_time, id);
    size_t index = expiration_heap_.size() - 1;
    
    key_to_index_[key] = index;
    key_to_id_[key] = id;
    
    // Restore heap property
    heapify_up(index);
    
    LOG_DEBUG(format_log("Set TTL for key '", key, "' to ", seconds, " seconds"));
    return true;
}

bool TTLManager::set_ttl_ms(const std::string& key, int64_t milliseconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (milliseconds <= 0) {
        // Remove TTL
        return remove_ttl(key);
    }
    
    auto expire_time = calculate_expire_time_ms(milliseconds);
    uint64_t id = next_id_++;
    
    // Check if key already has TTL
    auto it = key_to_index_.find(key);
    if (it != key_to_index_.end()) {
        // Update existing TTL
        size_t index = it->second;
        expiration_heap_[index].expire_time = expire_time;
        expiration_heap_[index].id = id;
        
        // Update ID mapping
        key_to_id_[key] = id;
        
        // Restore heap property
        heapify_up(index);
        heapify_down(index);
        
        LOG_DEBUG(format_log("Updated TTL for key '", key, "' to ", milliseconds, " ms"));
        return true;
    }
    
    // Add new TTL entry
    expiration_heap_.emplace_back(key, expire_time, id);
    size_t index = expiration_heap_.size() - 1;
    
    key_to_index_[key] = index;
    key_to_id_[key] = id;
    
    // Restore heap property
    heapify_up(index);
    
    LOG_DEBUG(format_log("Set TTL for key '", key, "' to ", milliseconds, " ms"));
    return true;
}

int64_t TTLManager::get_ttl(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = key_to_index_.find(key);
    if (it == key_to_index_.end()) {
        return -1;  // No TTL set
    }
    
    size_t index = it->second;
    auto expire_time = expiration_heap_[index].expire_time;
    auto now_time = now();
    
    if (expire_time <= now_time) {
        return -2;  // Key has expired
    }
    
    auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
        expire_time - now_time).count();
    
    return remaining;
}

int64_t TTLManager::get_ttl_ms(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = key_to_index_.find(key);
    if (it == key_to_index_.end()) {
        return -1;  // No TTL set
    }
    
    size_t index = it->second;
    auto expire_time = expiration_heap_[index].expire_time;
    auto now_time = now();
    
    if (expire_time <= now_time) {
        return -2;  // Key has expired
    }
    
    auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
        expire_time - now_time).count();
    
    return remaining;
}

bool TTLManager::has_ttl(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return key_to_index_.find(key) != key_to_index_.end();
}

bool TTLManager::remove_ttl(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = key_to_index_.find(key);
    if (it == key_to_index_.end()) {
        return false;  // No TTL to remove
    }
    
    size_t index = it->second;
    remove_from_heap(index);
    
    LOG_DEBUG(format_log("Removed TTL for key '", key, "'"));
    return true;
}

size_t TTLManager::check_expirations() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t expired_count = 0;
    auto now_time = now();
    
    // Check all expired entries
    while (!expiration_heap_.empty()) {
        const auto& top = expiration_heap_[0];
        
        if (top.expire_time > now_time) {
            // No more expired entries
            break;
        }
        
        // Key has expired
        std::string expired_key = top.key;
        
        // Remove from heap
        remove_from_heap(0);
        
        // Call expiration callback
        if (expiration_callback_) {
            expiration_callback_(expired_key);
        }
        
        expired_count++;
        total_expired_++;
        
        LOG_DEBUG(format_log("Key '", expired_key, "' expired"));
    }
    
    if (expired_count > 0) {
        LOG_INFO(format_log("Expired ", expired_count, " keys"));
    }
    
    return expired_count;
}

size_t TTLManager::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return expiration_heap_.size();
}

void TTLManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    expiration_heap_.clear();
    key_to_index_.clear();
    key_to_id_.clear();
    
    LOG_INFO("Cleared all TTL entries");
}

TTLManager::Stats TTLManager::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Stats stats;
    stats.total_ttl_entries = expiration_heap_.size();
    stats.total_expired = total_expired_.load();
    stats.expired_this_check = 0;  // Would be set by caller
    
    // Calculate average TTL
    if (!expiration_heap_.empty()) {
        auto now_time = now();
        double total_ttl = 0.0;
        
        for (const auto& entry : expiration_heap_) {
            auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
                entry.expire_time - now_time).count();
            total_ttl += std::max(0.0, static_cast<double>(remaining));
        }
        
        stats.average_ttl_seconds = total_ttl / expiration_heap_.size();
    } else {
        stats.average_ttl_seconds = 0.0;
    }
    
    return stats;
}

// ============================================================================
// Heap Operations
// ============================================================================

void TTLManager::heapify_up(size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        
        if (expiration_heap_[index] > expiration_heap_[parent]) {
            break;  // Heap property satisfied
        }
        
        swap_entries(index, parent);
        index = parent;
    }
}

void TTLManager::heapify_down(size_t index) {
    while (true) {
        size_t left_child = 2 * index + 1;
        size_t right_child = 2 * index + 2;
        size_t smallest = index;
        
        if (left_child < expiration_heap_.size() && 
            expiration_heap_[left_child] > expiration_heap_[smallest]) {
            smallest = left_child;
        }
        
        if (right_child < expiration_heap_.size() && 
            expiration_heap_[right_child] > expiration_heap_[smallest]) {
            smallest = right_child;
        }
        
        if (smallest == index) {
            break;  // Heap property satisfied
        }
        
        swap_entries(index, smallest);
        index = smallest;
    }
}

void TTLManager::swap_entries(size_t i, size_t j) {
    if (i == j) return;
    
    // Swap heap entries
    std::swap(expiration_heap_[i], expiration_heap_[j]);
    
    // Update index mappings
    key_to_index_[expiration_heap_[i].key] = i;
    key_to_index_[expiration_heap_[j].key] = j;
}

void TTLManager::remove_from_heap(size_t index) {
    if (index >= expiration_heap_.size()) {
        return;
    }
    
    // Move last element to index
    size_t last_index = expiration_heap_.size() - 1;
    
    if (index != last_index) {
        swap_entries(index, last_index);
    }
    
    // Remove last element
    std::string removed_key = expiration_heap_.back().key;
    expiration_heap_.pop_back();
    
    // Update mappings
    key_to_index_.erase(removed_key);
    key_to_id_.erase(removed_key);
    
    // Restore heap property if we moved an element
    if (index != last_index && !expiration_heap_.empty()) {
        heapify_up(index);
        heapify_down(index);
    }
}

// ============================================================================
// TTLManagerSingleton Implementation
// ============================================================================

void TTLManagerSingleton::start_background_checker() {
    if (running_.load()) {
        return;
    }
    
    running_ = true;
    background_thread_ = std::thread([this]() {
        background_checker_loop();
    });
    
    LOG_INFO("Started TTL background checker");
}

void TTLManagerSingleton::stop_background_checker() {
    if (!running_.load()) {
        return;
    }
    
    running_ = false;
    
    if (background_thread_.joinable()) {
        background_thread_.join();
    }
    
    LOG_INFO("Stopped TTL background checker");
}

void TTLManagerSingleton::background_checker_loop() {
    LOG_INFO("TTL background checker started");
    
    while (running_.load()) {
        // Check for expirations every 100ms
        manager_.check_expirations();
        
        // Sleep for 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    LOG_INFO("TTL background checker stopped");
}

} // namespace scuffedredis
