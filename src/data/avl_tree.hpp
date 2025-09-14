#ifndef SCUFFEDREDIS_AVL_TREE_HPP
#define SCUFFEDREDIS_AVL_TREE_HPP

/**
 * AVL Tree implementation for ScuffedRedis.
 * 
 * Self-balancing binary search tree for sorted operations.
 * Used for implementing sorted sets (ZADD, ZRANGE, ZRANK).
 */

#include <memory>
#include <functional>
#include <vector>
#include <optional>

namespace scuffedredis {

/**
 * AVL Tree node.
 * Stores key-value pairs with balance factor for AVL rotations.
 */
template<typename K, typename V>
struct AVLNode {
    K key;
    V value;
    int height;
    std::shared_ptr<AVLNode> left;
    std::shared_ptr<AVLNode> right;
    
    AVLNode(const K& k, const V& v) 
        : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
};

/**
 * AVL Tree implementation.
 * 
 * Features:
 * - O(log n) insert, delete, search
 * - Automatic rebalancing
 * - Range queries
 * - Rank operations
 */
template<typename K, typename V, typename Compare = std::less<K>>
class AVLTree {
public:
    using NodePtr = std::shared_ptr<AVLNode<K, V>>;
    using Node = AVLNode<K, V>;
    
    AVLTree() : root_(nullptr), size_(0) {}
    ~AVLTree() = default;
    
    /**
     * Insert key-value pair.
     * Updates value if key already exists.
     * Returns true if new key was inserted.
     */
    bool insert(const K& key, const V& value) {
        bool inserted = false;
        root_ = insertNode(root_, key, value, inserted);
        if (inserted) size_++;
        return inserted;
    }
    
    /**
     * Remove key from tree.
     * Returns true if key was found and removed.
     */
    bool remove(const K& key) {
        bool removed = false;
        root_ = removeNode(root_, key, removed);
        if (removed) size_--;
        return removed;
    }
    
    /**
     * Find value by key.
     * Returns nullopt if key doesn't exist.
     */
    std::optional<V> find(const K& key) const {
        NodePtr node = findNode(root_, key);
        return node ? std::optional<V>(node->value) : std::nullopt;
    }
    
    /**
     * Check if key exists.
     */
    bool contains(const K& key) const {
        return findNode(root_, key) != nullptr;
    }
    
    /**
     * Get all key-value pairs in sorted order.
     */
    std::vector<std::pair<K, V>> inorder() const {
        std::vector<std::pair<K, V>> result;
        inorderTraversal(root_, result);
        return result;
    }
    
    /**
     * Get range of elements [start, end].
     * Both bounds are inclusive.
     */
    std::vector<std::pair<K, V>> range(const K& start, const K& end) const {
        std::vector<std::pair<K, V>> result;
        rangeQuery(root_, start, end, result);
        return result;
    }
    
    /**
     * Get rank of key (0-based index in sorted order).
     * Returns -1 if key doesn't exist.
     */
    int rank(const K& key) const {
        return getRank(root_, key);
    }
    
    /**
     * Get number of elements in tree.
     */
    size_t size() const { return size_; }
    
    /**
     * Check if tree is empty.
     */
    bool empty() const { return size_ == 0; }
    
    /**
     * Clear all elements.
     */
    void clear() {
        root_ = nullptr;
        size_ = 0;
    }
    
    /**
     * Get tree height.
     * Used for debugging and testing balance.
     */
    int height() const {
        return getHeight(root_);
    }

private:
    NodePtr root_;
    size_t size_;
    Compare comp_;
    
    // Helper functions
    int getHeight(const NodePtr& node) const {
        return node ? node->height : 0;
    }
    
    int getBalance(const NodePtr& node) const {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }
    
    void updateHeight(NodePtr& node) {
        if (node) {
            node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        }
    }
    
    // Rotation operations
    NodePtr rotateRight(NodePtr& y) {
        NodePtr x = y->left;
        NodePtr T2 = x->right;
        
        // Perform rotation
        x->right = y;
        y->left = T2;
        
        // Update heights
        updateHeight(y);
        updateHeight(x);
        
        return x;
    }
    
    NodePtr rotateLeft(NodePtr& x) {
        NodePtr y = x->right;
        NodePtr T2 = y->left;
        
        // Perform rotation
        y->left = x;
        x->right = T2;
        
        // Update heights
        updateHeight(x);
        updateHeight(y);
        
        return y;
    }
    
    // Insert with rebalancing
    NodePtr insertNode(NodePtr node, const K& key, const V& value, bool& inserted) {
        // Standard BST insertion
        if (!node) {
            inserted = true;
            return std::make_shared<Node>(key, value);
        }
        
        if (comp_(key, node->key)) {
            node->left = insertNode(node->left, key, value, inserted);
        } else if (comp_(node->key, key)) {
            node->right = insertNode(node->right, key, value, inserted);
        } else {
            // Key exists, update value
            node->value = value;
            inserted = false;
            return node;
        }
        
        // Update height
        updateHeight(node);
        
        // Get balance factor
        int balance = getBalance(node);
        
        // Left-Left case
        if (balance > 1 && comp_(key, node->left->key)) {
            return rotateRight(node);
        }
        
        // Right-Right case
        if (balance < -1 && comp_(node->right->key, key)) {
            return rotateLeft(node);
        }
        
        // Left-Right case
        if (balance > 1 && comp_(node->left->key, key)) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        
        // Right-Left case
        if (balance < -1 && comp_(key, node->right->key)) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        
        return node;
    }
    
    // Find minimum node in subtree
    NodePtr findMin(NodePtr node) const {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }
    
    // Remove with rebalancing
    NodePtr removeNode(NodePtr node, const K& key, bool& removed) {
        if (!node) {
            removed = false;
            return node;
        }
        
        if (comp_(key, node->key)) {
            node->left = removeNode(node->left, key, removed);
        } else if (comp_(node->key, key)) {
            node->right = removeNode(node->right, key, removed);
        } else {
            // Node to delete found
            removed = true;
            
            // Node with only one child or no child
            if (!node->left || !node->right) {
                NodePtr temp = node->left ? node->left : node->right;
                return temp;
            }
            
            // Node with two children
            NodePtr temp = findMin(node->right);
            node->key = temp->key;
            node->value = temp->value;
            node->right = removeNode(node->right, temp->key, removed);
        }
        
        // Update height
        updateHeight(node);
        
        // Get balance factor
        int balance = getBalance(node);
        
        // Left-Left case
        if (balance > 1 && getBalance(node->left) >= 0) {
            return rotateRight(node);
        }
        
        // Left-Right case
        if (balance > 1 && getBalance(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        
        // Right-Right case
        if (balance < -1 && getBalance(node->right) <= 0) {
            return rotateLeft(node);
        }
        
        // Right-Left case
        if (balance < -1 && getBalance(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        
        return node;
    }
    
    // Find node by key
    NodePtr findNode(const NodePtr& node, const K& key) const {
        if (!node) return nullptr;
        
        if (comp_(key, node->key)) {
            return findNode(node->left, key);
        } else if (comp_(node->key, key)) {
            return findNode(node->right, key);
        } else {
            return node;
        }
    }
    
    // Inorder traversal
    void inorderTraversal(const NodePtr& node, std::vector<std::pair<K, V>>& result) const {
        if (!node) return;
        
        inorderTraversal(node->left, result);
        result.emplace_back(node->key, node->value);
        inorderTraversal(node->right, result);
    }
    
    // Range query
    void rangeQuery(const NodePtr& node, const K& start, const K& end, 
                   std::vector<std::pair<K, V>>& result) const {
        if (!node) return;
        
        // If current node is in range
        if (!comp_(node->key, start) && !comp_(end, node->key)) {
            rangeQuery(node->left, start, end, result);
            result.emplace_back(node->key, node->value);
            rangeQuery(node->right, start, end, result);
        }
        // If current node is smaller than start, search right
        else if (comp_(node->key, start)) {
            rangeQuery(node->right, start, end, result);
        }
        // If current node is greater than end, search left
        else {
            rangeQuery(node->left, start, end, result);
        }
    }
    
    // Get rank of key
    int getRank(const NodePtr& node, const K& key) const {
        if (!node) return -1;
        
        if (comp_(key, node->key)) {
            return getRank(node->left, key);
        } else if (comp_(node->key, key)) {
            int leftSize = getSubtreeSize(node->left);
            int rightRank = getRank(node->right, key);
            return rightRank == -1 ? -1 : leftSize + 1 + rightRank;
        } else {
            return getSubtreeSize(node->left);
        }
    }
    
    // Get size of subtree
    int getSubtreeSize(const NodePtr& node) const {
        if (!node) return 0;
        return 1 + getSubtreeSize(node->left) + getSubtreeSize(node->right);
    }
};

} // namespace scuffedredis

#endif // SCUFFEDREDIS_AVL_TREE_HPP
