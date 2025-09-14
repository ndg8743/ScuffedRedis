#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace scuffedredis {
namespace utils {

// Simple logger for debugging and monitoring
// Used when we need to track what's happening in the server
class Logger {
public:
    enum Level {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3
    };

    static void setLevel(Level level) { s_level = level; }
    
    template<typename... Args>
    static void debug(const std::string& format, Args... args) {
        log(DEBUG, format, args...);
    }
    
    template<typename... Args>
    static void info(const std::string& format, Args... args) {
        log(INFO, format, args...);
    }
    
    template<typename... Args>
    static void warn(const std::string& format, Args... args) {
        log(WARN, format, args...);
    }
    
    template<typename... Args>
    static void error(const std::string& format, Args... args) {
        log(ERROR, format, args...);
    }

private:
    static Level s_level;
    
    template<typename... Args>
    static void log(Level level, const std::string& format, Args... args) {
        if (level < s_level) return;
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        const char* level_str[] = {"DEBUG", "INFO", "WARN", "ERROR"};
        std::cout << "[" << ss.str() << "] [" << level_str[level] << "] " 
                  << format << std::endl;
    }
};

} // namespace utils
} // namespace scuffedredis
