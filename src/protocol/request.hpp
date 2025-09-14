#pragma once

#include <string>
#include <vector>
#include <memory>

namespace scuffedredis {
namespace protocol {

// Redis-like request structure
// Need this because we need to parse and handle different command types
class Request {
public:
    enum Type {
        GET,
        SET,
        DEL,
        EXISTS,
        KEYS,
        PING,
        QUIT,
        UNKNOWN
    };
    
    Request() = default;
    explicit Request(const std::string& raw);
    
    // Parse raw request string
    bool parse(const std::string& raw);
    
    // Getters
    Type getType() const { return type_; }
    const std::vector<std::string>& getArgs() const { return args_; }
    const std::string& getRaw() const { return raw_; }
    
    // Check if request is valid
    bool isValid() const { return type_ != UNKNOWN; }
    
    // Get command name
    std::string getCommand() const;

private:
    Type type_;
    std::vector<std::string> args_;
    std::string raw_;
    
    // Parse command type from string
    Type parseCommandType(const std::string& cmd);
};

} // namespace protocol
} // namespace scuffedredis
