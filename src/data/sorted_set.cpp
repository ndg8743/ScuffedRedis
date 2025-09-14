#include "sorted_set.hpp"
#include <algorithm>
#include <numeric>

namespace scuffedredis {

// ============================================================================
// SortedSet Implementation
// ============================================================================

SortedSet::SortedSet() : size_(0) {
}

SortedSet::~SortedSet() {
    clear();
}

int SortedSet::zadd(const std::string& member, double score) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if member already exists
    auto it = scores_.find(member);
    if (it != scores_.end()) {
        // Member exists, remove old entry from tree
        double old_score = it->second;
        SortedSetEntry old_entry(old_score, member);
        tree_.remove(old_entry);
        
        // Update score
        it->second = score;
        
        // Add new entry to tree
        SortedSetEntry new_entry(score, member);
        tree_.insert(new_entry, true);
        
        return 0;  // Updated existing member
    }
    
    // Add new member
    scores_[member] = score;
    SortedSetEntry entry(score, member);
    tree_.insert(entry, true);
    size_++;
    
    return 1;  // Added new member
}

int SortedSet::zadd_multi(const std::vector<std::pair<std::string, double>>& items) {
    int added = 0;
    for (const auto& [member, score] : items) {
        added += zadd(member, score);
    }
    return added;
}

int SortedSet::zrem(const std::string& member) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = scores_.find(member);
    if (it == scores_.end()) {
        return 0;  // Member not found
    }
    
    // Remove from tree
    double score = it->second;
    SortedSetEntry entry(score, member);
    tree_.remove(entry);
    
    // Remove from hash map
    scores_.erase(it);
    size_--;
    
    return 1;  // Removed
}

int SortedSet::zrem_multi(const std::vector<std::string>& members) {
    int removed = 0;
    for (const auto& member : members) {
        removed += zrem(member);
    }
    return removed;
}

std::optional<double> SortedSet::zscore(const std::string& member) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = scores_.find(member);
    if (it != scores_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<int> SortedSet::zrank(const std::string& member) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = scores_.find(member);
    if (it == scores_.end()) {
        return std::nullopt;
    }
    
    double score = it->second;
    SortedSetEntry entry(score, member);
    return tree_.rank(entry);
}

std::optional<int> SortedSet::zrevrank(const std::string& member) const {
    auto rank_opt = zrank(member);
    if (rank_opt.has_value()) {
        return static_cast<int>(size_) - 1 - rank_opt.value();
    }
    return std::nullopt;
}

std::vector<std::pair<std::string, double>> SortedSet::zrange(
    int start, int stop, bool withScores) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (size_ == 0) {
        return {};
    }
    
    // Normalize indices
    start = normalize_index(start);
    stop = normalize_index(stop);
    
    // Validate range
    if (start < 0) start = 0;
    if (stop >= static_cast<int>(size_)) stop = size_ - 1;
    if (start > stop) return {};
    
    // Get all entries in order
    auto all_entries = tree_.inorder();
    
    // Extract range
    std::vector<std::pair<std::string, double>> result;
    for (int i = start; i <= stop && i < static_cast<int>(all_entries.size()); i++) {
        const auto& entry = all_entries[i].first;
        if (withScores) {
            result.emplace_back(entry.member, entry.score);
        } else {
            result.emplace_back(entry.member, 0.0);
        }
    }
    
    return result;
}

std::vector<std::pair<std::string, double>> SortedSet::zrevrange(
    int start, int stop, bool withScores) const {
    auto result = zrange(start, stop, withScores);
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<std::pair<std::string, double>> SortedSet::zrangebyscore(
    double min, double max, bool withScores) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create dummy entries for range query
    SortedSetEntry min_entry(min, "");
    SortedSetEntry max_entry(max, "\xff");  // Max possible string
    
    auto entries = tree_.range(min_entry, max_entry);
    
    std::vector<std::pair<std::string, double>> result;
    for (const auto& [entry, _] : entries) {
        if (entry.score >= min && entry.score <= max) {
            if (withScores) {
                result.emplace_back(entry.member, entry.score);
            } else {
                result.emplace_back(entry.member, 0.0);
            }
        }
    }
    
    return result;
}

size_t SortedSet::zcount(double min, double max) const {
    auto range = zrangebyscore(min, max, false);
    return range.size();
}

void SortedSet::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    tree_.clear();
    scores_.clear();
    size_ = 0;
}

SortedSet::Stats SortedSet::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Stats stats;
    stats.total_members = size_;
    stats.tree_height = tree_.height();
    
    if (size_ == 0) {
        stats.min_score = 0.0;
        stats.max_score = 0.0;
        stats.avg_score = 0.0;
    } else {
        auto all_entries = tree_.inorder();
        
        if (!all_entries.empty()) {
            stats.min_score = all_entries.front().first.score;
            stats.max_score = all_entries.back().first.score;
            
            double sum = 0.0;
            for (const auto& [entry, _] : all_entries) {
                sum += entry.score;
            }
            stats.avg_score = sum / size_;
        }
    }
    
    return stats;
}

int SortedSet::normalize_index(int index) const {
    if (index < 0) {
        return static_cast<int>(size_) + index;
    }
    return index;
}

// ============================================================================
// SortedSetManager Implementation
// ============================================================================

std::shared_ptr<SortedSet> SortedSetManager::get_or_create(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = sets_.find(key);
    if (it != sets_.end()) {
        return it->second;
    }
    
    auto new_set = std::make_shared<SortedSet>();
    sets_[key] = new_set;
    return new_set;
}

bool SortedSetManager::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sets_.erase(key) > 0;
}

bool SortedSetManager::exists(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sets_.find(key) != sets_.end();
}

std::vector<std::string> SortedSetManager::keys() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    result.reserve(sets_.size());
    
    for (const auto& [key, _] : sets_) {
        result.push_back(key);
    }
    
    return result;
}

void SortedSetManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    sets_.clear();
}

} // namespace scuffedredis
