#ifndef DIDDLEDOODLEDUEL_PROFILER_H
#define DIDDLEDOODLEDUEL_PROFILER_H

#include <chrono>
#include <string>
#include <unordered_map>
#include <iostream>
#include "logging/logger.h"

class SimpleProfiler {
public:
    static SimpleProfiler& getInstance() {
        static SimpleProfiler instance;
        return instance;
    }

    void startTimer(const std::string& name) {
        timers[name] = std::chrono::high_resolution_clock::now();
    }

    void endTimer(const std::string& name) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto it = timers.find(name);
        if (it != timers.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second);
            totalTimes[name] += duration.count();
            callCounts[name]++;
        }
    }

    void printResults() const {
        LOG_INFO_MSG("\n=== Performance Profile ===");
        for (const auto& [name, totalTime] : totalTimes) {
            auto avgTime = totalTime / static_cast<double>(callCounts.at(name));
            LOG_INFO_MSG("%s: %.2fμs avg (%zu calls)", 
                         name.c_str(), avgTime, callCounts.at(name));
        }
        LOG_INFO_MSG("===========================\n");
    }

    void reset() {
        totalTimes.clear();
        callCounts.clear();
        timers.clear();
    }

private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers;
    std::unordered_map<std::string, long long> totalTimes;
    std::unordered_map<std::string, int> callCounts;
};

#define PROFILE_SCOPE(name) \
    SimpleProfiler::getInstance().startTimer(name); \
    auto scope_guard_##__LINE__ = [&]() { SimpleProfiler::getInstance().endTimer(name); }; \
    (void)scope_guard_##__LINE__

#define PROFILE_END(name) SimpleProfiler::getInstance().endTimer(name)

#endif // DIDDLEDOODLEDUEL_PROFILER_H