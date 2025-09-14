#pragma once

#include <string>
#include <vector>

namespace scuffedredis {
namespace protocol {

// Redis-like response structure
// Used when we need to send structured responses back to clients
class Response {
public:
    enum Type {
        STRING,     // Simple string
        ERROR,      // Error message
        INTEGER,    // Integer value
        BULK,       // Bulk string
        ARRAY,      // Array of responses
        NIL         // Null value
    };
    
    Response() = default;
    explicit Response(const std::string& value, Type type = STRING);
    explicit Response(int value);
    explicit Response(const std::vector<Response>& array);
    
    // Create specific response types
    static Response ok() { return Response("OK"); }
    static Response error(const std::string& msg) { return Response(msg, ERROR); }
    static Response nil() { return Response("", NIL); }
    static Response pong() { return Response("PONG"); }
    
    // Serialize response to string
    std::string serialize() const;
    
    // Getters
    Type getType() const { return type_; }
    const std::string& getValue() const { return value_; }
    int getIntValue() const { return intValue_; }
    const std::vector<Response>& getArray() const { return array_; }

private:
    Type type_;
    std::string value_;
    int intValue_;
    std::vector<Response> array_;
    
    // Helper to escape strings
    std::string escapeString(const std::string& str) const;
};

} // namespace protocol
} // namespace scuffedredis
