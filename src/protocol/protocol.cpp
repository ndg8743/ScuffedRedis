#include "protocol.hpp"
#include <cstring>
#include <algorithm>

namespace scuffedredis {
namespace protocol {

// ============================================================================
// Message Implementation
// ============================================================================

MessagePtr Message::make_simple_string(const std::string& str) {
    auto msg = std::make_shared<Message>(MessageType::SIMPLE_STRING);
    msg->value_ = str;
    return msg;
}

MessagePtr Message::make_error(const std::string& error) {
    auto msg = std::make_shared<Message>(MessageType::ERROR_MSG);
    msg->value_ = error;
    return msg;
}

MessagePtr Message::make_integer(int64_t value) {
    auto msg = std::make_shared<Message>(MessageType::INTEGER);
    msg->value_ = value;
    return msg;
}

MessagePtr Message::make_bulk_string(const std::string& str) {
    auto msg = std::make_shared<Message>(MessageType::BULK_STRING);
    msg->value_ = str;
    return msg;
}

MessagePtr Message::make_array(const MessageArray& array) {
    auto msg = std::make_shared<Message>(MessageType::ARRAY);
    msg->value_ = array;
    return msg;
}

MessagePtr Message::make_null() {
    return std::make_shared<Message>(MessageType::NULL_VALUE);
}

std::string Message::as_string() const {
    if (auto* str = std::get_if<std::string>(&value_)) {
        return *str;
    }
    return "";
}

int64_t Message::as_integer() const {
    if (auto* val = std::get_if<int64_t>(&value_)) {
        return *val;
    }
    return 0;
}

const MessageArray* Message::as_array() const {
    if (auto* arr = std::get_if<MessageArray>(&value_)) {
        return arr;
    }
    return nullptr;
}

std::vector<uint8_t> Message::serialize() const {
    std::vector<uint8_t> result;
    
    // Add type byte
    result.push_back(static_cast<uint8_t>(type_));
    
    switch (type_) {
        case MessageType::SIMPLE_STRING:
        case MessageType::ERROR_MSG:
        case MessageType::BULK_STRING: {
            const std::string& str = as_string();
            uint32_t len = static_cast<uint32_t>(str.size());
            
            // Add length (4 bytes, little-endian)
            result.push_back(len & 0xFF);
            result.push_back((len >> 8) & 0xFF);
            result.push_back((len >> 16) & 0xFF);
            result.push_back((len >> 24) & 0xFF);
            
            // Add string data
            result.insert(result.end(), str.begin(), str.end());
            break;
        }
        
        case MessageType::INTEGER: {
            int64_t val = as_integer();
            
            // Length is always 8 for int64
            uint32_t len = 8;
            result.push_back(len & 0xFF);
            result.push_back((len >> 8) & 0xFF);
            result.push_back((len >> 16) & 0xFF);
            result.push_back((len >> 24) & 0xFF);
            
            // Add integer value (8 bytes, little-endian)
            for (int i = 0; i < 8; i++) {
                result.push_back((val >> (i * 8)) & 0xFF);
            }
            break;
        }
        
        case MessageType::ARRAY: {
            auto arr = as_array();
            uint32_t count = arr ? static_cast<uint32_t>(arr->size()) : 0;
            
            // Add array count as length
            result.push_back(count & 0xFF);
            result.push_back((count >> 8) & 0xFF);
            result.push_back((count >> 16) & 0xFF);
            result.push_back((count >> 24) & 0xFF);
            
            // Recursively serialize each element
            if (arr) {
                for (const auto& elem : *arr) {
                    if (elem) {
                        auto elem_data = elem->serialize();
                        result.insert(result.end(), elem_data.begin(), elem_data.end());
                    } else {
                        // Null element - serialize as NULL_VALUE
                        auto null_msg = make_null();
                        auto null_data = null_msg->serialize();
                        result.insert(result.end(), null_data.begin(), null_data.end());
                    }
                }
            }
            break;
        }
        
        case MessageType::NULL_VALUE: {
            // Length is 0 for null
            result.push_back(0);
            result.push_back(0);
            result.push_back(0);
            result.push_back(0);
            break;
        }
    }
    
    return result;
}

size_t Message::serialized_size() const {
    // Type (1) + Length (4) + Data
    size_t size = 5;
    
    switch (type_) {
        case MessageType::SIMPLE_STRING:
        case MessageType::ERROR_MSG:
        case MessageType::BULK_STRING:
            size += as_string().size();
            break;
            
        case MessageType::INTEGER:
            size += 8;  // int64_t
            break;
            
        case MessageType::ARRAY:
            if (auto arr = as_array()) {
                for (const auto& elem : *arr) {
                    size += elem ? elem->serialized_size() : 5;  // NULL is 5 bytes
                }
            }
            break;
            
        case MessageType::NULL_VALUE:
            // Already included in header
            break;
    }
    
    return size;
}

// ============================================================================
// Parser Implementation
// ============================================================================

Parser::Parser() {
    buffer_.reserve(4096);  // Reserve initial space for efficiency
}

Parser::~Parser() = default;

void Parser::feed(const uint8_t* data, size_t size) {
    buffer_.insert(buffer_.end(), data, data + size);
}

void Parser::feed(const std::vector<uint8_t>& data) {
    feed(data.data(), data.size());
}

bool Parser::has_message() const {
    // Need at least 5 bytes for header (type + length)
    if (buffer_.size() < 5) {
        return false;
    }
    
    // Read length to check if we have complete message
    uint32_t length = 0;
    length |= buffer_[1];
    length |= buffer_[2] << 8;
    length |= buffer_[3] << 16;
    length |= buffer_[4] << 24;
    
    // Check if it's an array (special case)
    MessageType type = static_cast<MessageType>(buffer_[0]);
    if (type == MessageType::ARRAY) {
        // For arrays, length is count, need to parse recursively
        // This is a simplified check
        return true;  // Let parse_message handle the details
    }
    
    // Check if we have the complete message
    return buffer_.size() >= 5 + length;
}

MessagePtr Parser::parse_message() {
    MessageType type;
    uint32_t length;
    
    if (!read_header(type, length)) {
        return nullptr;
    }
    
    switch (type) {
        case MessageType::SIMPLE_STRING:
            return parse_simple_string(length);
            
        case MessageType::ERROR_MSG:
            return parse_error(length);
            
        case MessageType::INTEGER:
            return parse_integer(length);
            
        case MessageType::BULK_STRING:
            return parse_bulk_string(length);
            
        case MessageType::ARRAY:
            return parse_array(length);  // length is count for arrays
            
        case MessageType::NULL_VALUE:
            consume_bytes(5);  // Just the header
            return Message::make_null();
            
        default:
            // Unknown type, clear buffer to recover
            reset();
            return nullptr;
    }
}

bool Parser::read_header(MessageType& type, uint32_t& length) {
    if (!has_bytes(5)) {
        return false;
    }
    
    type = static_cast<MessageType>(buffer_[0]);
    
    // Read length (little-endian)
    length = 0;
    length |= buffer_[1];
    length |= buffer_[2] << 8;
    length |= buffer_[3] << 16;
    length |= buffer_[4] << 24;
    
    return true;
}

MessagePtr Parser::parse_simple_string(uint32_t length) {
    if (!has_bytes(5 + length)) {
        return nullptr;
    }
    
    std::string str(buffer_.begin() + 5, buffer_.begin() + 5 + length);
    consume_bytes(5 + length);
    
    return Message::make_simple_string(str);
}

MessagePtr Parser::parse_error(uint32_t length) {
    if (!has_bytes(5 + length)) {
        return nullptr;
    }
    
    std::string str(buffer_.begin() + 5, buffer_.begin() + 5 + length);
    consume_bytes(5 + length);
    
    return Message::make_error(str);
}

MessagePtr Parser::parse_integer(uint32_t length) {
    if (length != 8 || !has_bytes(5 + 8)) {
        return nullptr;
    }
    
    // Read int64_t (little-endian)
    int64_t value = 0;
    for (int i = 0; i < 8; i++) {
        value |= static_cast<int64_t>(buffer_[5 + i]) << (i * 8);
    }
    
    consume_bytes(5 + 8);
    return Message::make_integer(value);
}

MessagePtr Parser::parse_bulk_string(uint32_t length) {
    if (!has_bytes(5 + length)) {
        return nullptr;
    }
    
    std::string str(buffer_.begin() + 5, buffer_.begin() + 5 + length);
    consume_bytes(5 + length);
    
    return Message::make_bulk_string(str);
}

MessagePtr Parser::parse_array(uint32_t count) {
    // Consume array header
    consume_bytes(5);
    
    MessageArray array;
    array.reserve(count);
    
    // Parse each element
    for (uint32_t i = 0; i < count; i++) {
        auto elem = parse_message();
        if (!elem) {
            // Not enough data, restore buffer
            // This is simplified - proper implementation would track position
            return nullptr;
        }
        array.push_back(elem);
    }
    
    return Message::make_array(array);
}

void Parser::consume_bytes(size_t count) {
    if (count >= buffer_.size()) {
        buffer_.clear();
    } else {
        buffer_.erase(buffer_.begin(), buffer_.begin() + count);
    }
}

bool Parser::has_bytes(size_t count) const {
    return buffer_.size() >= count;
}

void Parser::reset() {
    buffer_.clear();
}

// ============================================================================
// Protocol Utilities
// ============================================================================

namespace utils {

MessagePtr make_command(const std::vector<std::string>& args) {
    MessageArray array;
    array.reserve(args.size());
    
    // Convert each argument to bulk string
    for (const auto& arg : args) {
        array.push_back(Message::make_bulk_string(arg));
    }
    
    return Message::make_array(array);
}

std::vector<std::string> parse_command(const MessagePtr& msg) {
    std::vector<std::string> result;
    
    if (!msg || !msg->is_array()) {
        return result;
    }
    
    auto arr = msg->as_array();
    if (!arr) {
        return result;
    }
    
    result.reserve(arr->size());
    
    // Extract string from each element
    for (const auto& elem : *arr) {
        if (elem && elem->is_string()) {
            result.push_back(elem->as_string());
        } else {
            // Invalid command format
            result.clear();
            break;
        }
    }
    
    return result;
}

MessagePtr ok_response() {
    return Message::make_simple_string("OK");
}

MessagePtr pong_response() {
    return Message::make_simple_string("PONG");
}

MessagePtr nil_response() {
    return Message::make_null();
}

MessagePtr error_response(const std::string& error) {
    return Message::make_error(error);
}

std::vector<uint8_t> serialize_messages(const std::vector<MessagePtr>& messages) {
    std::vector<uint8_t> result;
    
    // Pre-calculate total size for efficiency
    size_t total_size = 0;
    for (const auto& msg : messages) {
        if (msg) {
            total_size += msg->serialized_size();
        }
    }
    
    result.reserve(total_size);
    
    // Serialize each message
    for (const auto& msg : messages) {
        if (msg) {
            auto data = msg->serialize();
            result.insert(result.end(), data.begin(), data.end());
        }
    }
    
    return result;
}

} // namespace utils
} // namespace protocol
} // namespace scuffedredis
