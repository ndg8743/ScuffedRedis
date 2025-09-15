#ifndef SCUFFEDREDIS_LOGGER_HPP
#define SCUFFEDREDIS_LOGGER_HPP

/**
 * Simple logging utility for ScuffedRedis.
 * 
 * Provides thread-safe logging with different severity levels.
 * Designed for simplicity and performance.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace scuffedredis {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR_LEVEL = 3,
    FATAL = 4
};

/**
 * Simple logger class.
 * Thread-safe and efficient for basic logging needs.
 */
class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }
    
    // Set minimum log level
    void set_level(LogLevel level) {
        min_level_ = level;
    }
    
    // Enable/disable timestamp
    void set_show_timestamp(bool show) {
        show_timestamp_ = show;
    }
    
    // Log methods
    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg) { log(LogLevel::INFO, msg); }
    void warn(const std::string& msg) { log(LogLevel::WARN, msg); }
    void error(const std::string& msg) { log(LogLevel::ERROR_LEVEL, msg); }
    void fatal(const std::string& msg) { log(LogLevel::FATAL, msg); }
    
    // Generic log method
    void log(LogLevel level, const std::string& msg) {
        if (level < min_level_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Add timestamp if enabled
        if (show_timestamp_) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::cerr << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] ";
        }
        
        // Add level prefix
        std::cerr << "[" << level_string(level) << "] ";
        
        // Output message
        std::cerr << msg << std::endl;
    }

private:
    Logger() : min_level_(LogLevel::INFO), show_timestamp_(true) {}
    
    std::mutex mutex_;
    LogLevel min_level_;
    bool show_timestamp_;
    
    const char* level_string(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR_LEVEL: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
};

// Convenience macros for logging
// These capture file and line information for debugging
#define LOG_DEBUG(msg) \
    scuffedredis::Logger::instance().debug(msg)

#define LOG_INFO(msg) \
    scuffedredis::Logger::instance().info(msg)

#define LOG_WARN(msg) \
    scuffedredis::Logger::instance().warn(msg)

#define LOG_ERROR(msg) \
    scuffedredis::Logger::instance().error(msg)

#define LOG_FATAL(msg) \
    scuffedredis::Logger::instance().fatal(msg)

// Helper for formatting log messages
template<typename... Args>
std::string format_log(Args... args) {
    std::ostringstream oss;
    ((oss << args), ...);
    return oss.str();
}

} // namespace scuffedredis

#endif // SCUFFEDREDIS_LOGGER_HPP
