#include "response.hpp"
#include <sstream>

namespace scuffedredis {
namespace protocol {

Response::Response(const std::string& value, Type type) 
    : type_(type), value_(value), intValue_(0) {
}

Response::Response(int value) 
    : type_(INTEGER), intValue_(value) {
}

Response::Response(const std::vector<Response>& array) 
    : type_(ARRAY), array_(array) {
}

std::string Response::serialize() const {
    std::ostringstream oss;
    
    switch (type_) {
        case STRING:
            oss << "+" << value_ << "\r\n";
            break;
            
        case ERROR:
            oss << "-" << value_ << "\r\n";
            break;
            
        case INTEGER:
            oss << ":" << intValue_ << "\r\n";
            break;
            
        case BULK:
            if (value_.empty()) {
                oss << "$-1\r\n";  // Null bulk string
            } else {
                oss << "$" << value_.length() << "\r\n" << value_ << "\r\n";
            }
            break;
            
        case ARRAY:
            oss << "*" << array_.size() << "\r\n";
            for (const auto& item : array_) {
                oss << item.serialize();
            }
            break;
            
        case NIL:
            oss << "$-1\r\n";
            break;
    }
    
    return oss.str();
}

std::string Response::escapeString(const std::string& str) const {
    std::string escaped;
    escaped.reserve(str.length() * 2);
    
    for (char c : str) {
        switch (c) {
            case '\r':
                escaped += "\\r";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\t':
                escaped += "\\t";
                break;
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            default:
                escaped += c;
                break;
        }
    }
    
    return escaped;
}

} // namespace protocol
} // namespace scuffedredis
