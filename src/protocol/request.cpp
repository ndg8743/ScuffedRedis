#include "request.hpp"
#include "../utils/logger.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace scuffedredis {
namespace protocol {

Request::Request(const std::string& raw) {
    parse(raw);
}

bool Request::parse(const std::string& raw) {
    raw_ = raw;
    args_.clear();
    type_ = UNKNOWN;
    
    if (raw.empty()) {
        return false;
    }
    
    // Simple parsing - split by spaces and convert to uppercase
    std::istringstream iss(raw);
    std::string token;
    
    while (iss >> token) {
        // Convert to uppercase for command matching
        std::transform(token.begin(), token.end(), token.begin(), ::toupper);
        args_.push_back(token);
    }
    
    if (args_.empty()) {
        return false;
    }
    
    // Parse command type
    type_ = parseCommandType(args_[0]);
    
    utils::Logger::debug("Parsed request: {} -> {} args", args_[0], args_.size());
    return type_ != UNKNOWN;
}

Request::Type Request::parseCommandType(const std::string& cmd) {
    if (cmd == "GET") return GET;
    if (cmd == "SET") return SET;
    if (cmd == "DEL") return DEL;
    if (cmd == "EXISTS") return EXISTS;
    if (cmd == "KEYS") return KEYS;
    if (cmd == "PING") return PING;
    if (cmd == "QUIT") return QUIT;
    
    return UNKNOWN;
}

std::string Request::getCommand() const {
    if (args_.empty()) {
        return "";
    }
    return args_[0];
}

} // namespace protocol
} // namespace scuffedredis
