#include "request_handler.hpp"
#include "../utils/logger.hpp"

namespace scuffedredis {
namespace protocol {

RequestHandler::RequestHandler() {
    utils::Logger::info("Request handler initialized");
}

Response RequestHandler::handleRequest(const Request& request) {
    if (!request.isValid()) {
        return Response::error("Unknown command");
    }
    
    utils::Logger::debug("Handling request: {}", request.getCommand());
    
    switch (request.getType()) {
        case Request::GET:
            return handleGet(request);
        case Request::SET:
            return handleSet(request);
        case Request::DEL:
            return handleDel(request);
        case Request::EXISTS:
            return handleExists(request);
        case Request::KEYS:
            return handleKeys(request);
        case Request::PING:
            return handlePing(request);
        case Request::QUIT:
            return handleQuit(request);
        default:
            return Response::error("Unknown command");
    }
}

Response RequestHandler::handleGet(const Request& request) {
    if (!validateArgs(request, 2, "GET")) {
        return Response::error("Wrong number of arguments for GET command");
    }
    
    const auto& args = request.getArgs();
    std::string value;
    
    if (storage_.get(args[1], value)) {
        return Response(value, Response::BULK);
    } else {
        return Response::nil();
    }
}

Response RequestHandler::handleSet(const Request& request) {
    if (!validateArgs(request, 3, "SET")) {
        return Response::error("Wrong number of arguments for SET command");
    }
    
    const auto& args = request.getArgs();
    storage_.insert(args[1], args[2]);
    
    utils::Logger::debug("Set key '{}' to value '{}'", args[1], args[2]);
    return Response::ok();
}

Response RequestHandler::handleDel(const Request& request) {
    if (!validateArgs(request, 2, "DEL")) {
        return Response::error("Wrong number of arguments for DEL command");
    }
    
    const auto& args = request.getArgs();
    bool removed = storage_.remove(args[1]);
    
    return Response(removed ? 1 : 0);
}

Response RequestHandler::handleExists(const Request& request) {
    if (!validateArgs(request, 2, "EXISTS")) {
        return Response::error("Wrong number of arguments for EXISTS command");
    }
    
    const auto& args = request.getArgs();
    bool exists = storage_.exists(args[1]);
    
    return Response(exists ? 1 : 0);
}

Response RequestHandler::handleKeys(const Request& request) {
    if (!validateArgs(request, 1, "KEYS")) {
        return Response::error("Wrong number of arguments for KEYS command");
    }
    
    auto keys = storage_.getAllKeys();
    std::vector<Response> keyResponses;
    
    for (const auto& key : keys) {
        keyResponses.emplace_back(key, Response::BULK);
    }
    
    return Response(keyResponses);
}

Response RequestHandler::handlePing(const Request& request) {
    return Response::pong();
}

Response RequestHandler::handleQuit(const Request& request) {
    return Response("OK");
}

bool RequestHandler::validateArgs(const Request& request, size_t expectedCount, const std::string& command) {
    if (request.getArgs().size() != expectedCount) {
        utils::Logger::warn("Invalid argument count for {}: expected {}, got {}", 
                           command, expectedCount, request.getArgs().size());
        return false;
    }
    return true;
}

} // namespace protocol
} // namespace scuffedredis
