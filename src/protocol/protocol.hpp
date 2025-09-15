#ifndef SCUFFEDREDIS_PROTOCOL_HPP
#define SCUFFEDREDIS_PROTOCOL_HPP

/**
 * ScuffedRedis Protocol Definition
 * 
 * Defines the binary protocol for client-server communication.
 * Similar to Redis RESP but simplified for learning purposes.
 * 
 * Protocol Format:
 * [Type:1 byte][Length:4 bytes][Data:N bytes]
 * 
 * Type codes:
 * - 0x01: Simple String (OK, PONG, etc.)
 * - 0x02: Error
 * - 0x03: Integer
 * - 0x04: Bulk String (binary safe)
 * - 0x05: Array
 * - 0x06: Null
 */

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace scuffedredis {
namespace protocol {

// Protocol constants
enum class MessageType : uint8_t {
    SIMPLE_STRING = 0x01,  // Simple string response
    ERROR_MSG = 0x02,      // Error message
    INTEGER = 0x03,        // Integer value
    BULK_STRING = 0x04,    // Binary-safe string
    ARRAY = 0x05,          // Array of messages
    NULL_VALUE = 0x06      // Null/nil response
};

// Forward declarations
class Message;
using MessagePtr = std::shared_ptr<Message>;
using MessageArray = std::vector<MessagePtr>;

/**
 * Base message class for protocol messages.
 * Uses variant to hold different value types.
 */
class Message {
public:
    using Value = std::variant<
        std::monostate,     // NULL_VALUE
        std::string,        // SIMPLE_STRING, ERROR, BULK_STRING
        int64_t,            // INTEGER
        MessageArray        // ARRAY
    >;
    
    Message() : type_(MessageType::NULL_VALUE) {}
    explicit Message(MessageType type) : type_(type) {}
    
    // Factory methods for creating messages
    static MessagePtr make_simple_string(const std::string& str);
    static MessagePtr make_error(const std::string& error);
    static MessagePtr make_integer(int64_t value);
    static MessagePtr make_bulk_string(const std::string& str);
    static MessagePtr make_array(const MessageArray& array);
    static MessagePtr make_null();
    
    // Getters
    MessageType get_type() const { return type_; }
    const Value& get_value() const { return value_; }
    
    // Type checking helpers
    bool is_string() const { 
        return type_ == MessageType::SIMPLE_STRING || 
               type_ == MessageType::BULK_STRING;
    }
    bool is_error() const { return type_ == MessageType::ERROR_MSG; }
    bool is_integer() const { return type_ == MessageType::INTEGER; }
    bool is_array() const { return type_ == MessageType::ARRAY; }
    bool is_null() const { return type_ == MessageType::NULL_VALUE; }
    
    // Value extraction helpers (returns nullptr/0/"" if wrong type)
    std::string as_string() const;
    int64_t as_integer() const;
    const MessageArray* as_array() const;
    
    // Serialize message to binary format
    std::vector<uint8_t> serialize() const;
    
    // Get size of serialized message
    size_t serialized_size() const;

private:
    MessageType type_;
    Value value_;
    
    friend class Parser;
    void set_value(const Value& val) { value_ = val; }
};

/**
 * Protocol parser for deserializing messages.
 * Handles partial messages and buffering.
 */
class Parser {
public:
    Parser();
    ~Parser();
    
    /**
     * Feed data to the parser.
     * Data is buffered internally for partial message handling.
     */
    void feed(const uint8_t* data, size_t size);
    void feed(const std::vector<uint8_t>& data);
    
    /**
     * Try to parse a complete message.
     * Returns nullptr if no complete message is available.
     * Call repeatedly to get all available messages.
     */
    MessagePtr parse_message();
    
    /**
     * Check if a complete message is available.
     */
    bool has_message() const;
    
    /**
     * Clear internal buffer.
     */
    void reset();
    
    /**
     * Get current buffer size (for debugging/monitoring).
     */
    size_t buffer_size() const { return buffer_.size(); }

private:
    std::vector<uint8_t> buffer_;  // Internal buffer for partial messages
    
    // Helper methods for parsing
    bool read_header(MessageType& type, uint32_t& length);
    MessagePtr parse_simple_string(uint32_t length);
    MessagePtr parse_error(uint32_t length);
    MessagePtr parse_integer(uint32_t length);
    MessagePtr parse_bulk_string(uint32_t length);
    MessagePtr parse_array(uint32_t count);
    
    // Buffer management
    void consume_bytes(size_t count);
    bool has_bytes(size_t count) const;
};

/**
 * High-level protocol utilities.
 */
namespace utils {
    
    /**
     * Create a command message from strings.
     * Example: make_command({"SET", "key", "value"})
     */
    MessagePtr make_command(const std::vector<std::string>& args);
    
    /**
     * Parse command from array message.
     * Returns empty vector if not a valid command.
     */
    std::vector<std::string> parse_command(const MessagePtr& msg);
    
    /**
     * Create standard response messages.
     */
    MessagePtr ok_response();
    MessagePtr pong_response();
    MessagePtr nil_response();
    MessagePtr error_response(const std::string& error);
    
    /**
     * Serialize multiple messages into a single buffer.
     * Useful for pipelining.
     */
    std::vector<uint8_t> serialize_messages(const std::vector<MessagePtr>& messages);
}

} // namespace protocol
} // namespace scuffedredis

#endif // SCUFFEDREDIS_PROTOCOL_HPP
