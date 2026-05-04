#pragma once

#include <chrono>

namespace airwatcher::utils {

class ScopedTimer {
public:
    ScopedTimer() : start_(std::chrono::steady_clock::now()) {}

    long long elapsedMs() const {
        const auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

private:
    std::chrono::steady_clock::time_point start_;
};

} // namespace airwatcher::utils
