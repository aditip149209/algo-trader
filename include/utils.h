// ============================================================================
// include/utils.h
// Utility functions for timing, logging, and helpers
// ============================================================================

#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>

// Simple timer class for performance measurement
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    Timer() {
        reset();
    }
    
    void reset() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    // Returns elapsed time in milliseconds
    double elapsed_ms() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(
            end_time - start_time
        ).count();
    }
    
    // Returns elapsed time in seconds
    double elapsed_s() const {
        return elapsed_ms() / 1000.0;
    }
};

// Logging utilities
namespace Logger {
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
}

#endif // UTILS_H
