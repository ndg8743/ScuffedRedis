#pragma once

#include "request.hpp"
#include "response.hpp"
#include "../data/hashtable.hpp"
#include <memory>

namespace scuffedredis {
namespace protocol {

// Handles incoming requests and generates responses
// Used when we need to process client commands and return appropriate responses
class RequestHandler {
public:
    RequestHandler();
    ~RequestHandler() = default;
    
    // Process a request and return response
    Response handleRequest(const Request& request);
    
    // Get storage statistics
    size_t getKeyCount() const { return storage_.size(); }

private:
    // Storage for key-value pairs
    data::HashTable<std::string, std::string> storage_;
    
    // Command handlers
    Response handleGet(const Request& request);
    Response handleSet(const Request& request);
    Response handleDel(const Request& request);
    Response handleExists(const Request& request);
    Response handleKeys(const Request& request);
    Response handlePing(const Request& request);
    Response handleQuit(const Request& request);
    
    // Helper to validate argument count
    bool validateArgs(const Request& request, size_t expectedCount, const std::string& command);
};

} // namespace protocol
} // namespace scuffedredis
