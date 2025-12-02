#pragma once
#include <chrono>

class Timer {
public:
    void reset() { start_ = std::chrono::high_resolution_clock::now(); }
    long long elapsed() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start_).count();
    }
    void setInterval(long ms) { interval_ = ms; }
private:
    std::chrono::high_resolution_clock::time_point start_;
    long interval_ = 16; // 60FPS
};
