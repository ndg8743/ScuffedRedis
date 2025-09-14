#ifndef SCUFFEDREDIS_SORTED_SET_HPP
#define SCUFFEDREDIS_SORTED_SET_HPP

/**
 * Sorted Set implementation for ScuffedRedis.
 * 
 * Implements Redis sorted set commands using AVL tree.
 * Supports ZADD, ZRANGE, ZRANK, ZREM, ZSCORE operations.
 */

#include "avl_tree.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>
#include <mutex>

namespace scuffedredis {

/**
 * Sorted Set entry combining score and member.
 * Used as key in AVL tree for sorting.
 */
struct SortedSetEntry {
    double score;
    std::string member;
    
    SortedSetEntry(double s, const std::string& m) : score(s), member(m) {}
    
    // Comparison for AVL tree ordering
    // First by score, then by member (lexicographic)
    bool operator<(const SortedSetEntry& other) const {
        if (score != other.score) {
            return score < other.score;
        }
        return member < other.member;
    }
};

/**
 * Sorted Set implementation.
 * 
 * Uses AVL tree for sorted storage and hash map for O(1) score lookups.
 * Thread-safe with mutex protection.
 */
class SortedSet {
public:
    SortedSet();
    ~SortedSet();
    
    /**
     * Add member with score to sorted set.
     * Updates score if member already exists.
     * Returns 1 if new member added, 0 if updated.
     */
    int zadd(const std::string& member, double score);
    
    /**
     * Add multiple members with scores.
     * Returns number of new members added.
     */
    int zadd_multi(const std::vector<std::pair<std::string, double>>& items);
    
    /**
     * Remove member from sorted set.
     * Returns 1 if removed, 0 if not found.
     */
    int zrem(const std::string& member);
    
    /**
     * Remove multiple members.
     * Returns number of members removed.
     */
    int zrem_multi(const std::vector<std::string>& members);
    
    /**
     * Get score of member.
     * Returns nullopt if member doesn't exist.
     */
    std::optional<double> zscore(const std::string& member) const;
    
    /**
     * Get rank of member (0-based, ascending order).
     * Returns nullopt if member doesn't exist.
     */
    std::optional<int> zrank(const std::string& member) const;
    
    /**
     * Get reverse rank of member (0-based, descending order).
     * Returns nullopt if member doesn't exist.
     */
    std::optional<int> zrevrank(const std::string& member) const;
    
    /**
     * Get range of members by rank [start, stop].
     * Supports negative indices (-1 = last element).
     * withScores: include scores in result.
     */
    std::vector<std::pair<std::string, double>> zrange(
        int start, int stop, bool withScores = false) const;
    
    /**
     * Get reverse range of members by rank [start, stop].
     * Same as zrange but in descending order.
     */
    std::vector<std::pair<std::string, double>> zrevrange(
        int start, int stop, bool withScores = false) const;
    
    /**
     * Get range of members by score [min, max].
     * Both bounds are inclusive.
     */
    std::vector<std::pair<std::string, double>> zrangebyscore(
        double min, double max, bool withScores = false) const;
    
    /**
     * Count members with scores in range [min, max].
     */
    size_t zcount(double min, double max) const;
    
    /**
     * Get number of members in sorted set.
     */
    size_t zcard() const { return size_; }
    
    /**
     * Check if sorted set is empty.
     */
    bool empty() const { return size_ == 0; }
    
    /**
     * Clear all members.
     */
    void clear();
    
    /**
     * Get statistics about the sorted set.
     */
    struct Stats {
        size_t total_members;
        double min_score;
        double max_score;
        double avg_score;
        int tree_height;
    };
    
    Stats get_stats() const;

private:
    // AVL tree for sorted storage
    AVLTree<SortedSetEntry, bool> tree_;
    
    // Hash map for O(1) score lookups
    std::unordered_map<std::string, double> scores_;
    
    // Size tracking
    size_t size_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    /**
     * Normalize index for range operations.
     * Handles negative indices.
     */
    int normalize_index(int index) const;
    
    /**
     * Convert tree entries to result format.
     */
    std::vector<std::pair<std::string, double>> format_results(
        const std::vector<std::pair<SortedSetEntry, bool>>& entries,
        bool withScores) const;
};

/**
 * Thread-safe sorted set manager.
 * Manages multiple sorted sets by key.
 */
class SortedSetManager {
public:
    SortedSetManager() = default;
    ~SortedSetManager() = default;
    
    /**
     * Get or create sorted set by key.
     */
    std::shared_ptr<SortedSet> get_or_create(const std::string& key);
    
    /**
     * Delete sorted set by key.
     * Returns true if deleted, false if not found.
     */
    bool del(const std::string& key);
    
    /**
     * Check if sorted set exists.
     */
    bool exists(const std::string& key) const;
    
    /**
     * Get all sorted set keys.
     */
    std::vector<std::string> keys() const;
    
    /**
     * Clear all sorted sets.
     */
    void clear();

private:
    std::unordered_map<std::string, std::shared_ptr<SortedSet>> sets_;
    mutable std::mutex mutex_;
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_SORTED_SET_HPP
