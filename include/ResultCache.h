#pragma once

#include "LogAnalyzer.h"
#include <string>
#include <unordered_map>

// Maps a mode (e.g., "serial") to its statistics
using ModeResults = std::unordered_map<std::string, LogStatistics>;

// Maps a log file path to its collection of mode results
using ResultsCache = std::unordered_map<std::string, ModeResults>;

namespace ResultCacheManager {

    const std::string CACHE_FILE_PATH = "build/comparison_cache.json";

    // Load the entire cache from the JSON file
    ResultsCache load();

    // Save the entire cache to the JSON file
    void save(const ResultsCache& cache);

    // Update the cache with a new result and save it
    void updateAndSave(const std::string& logFilePath, const std::string& mode, const LogStatistics& stats);

} // namespace ResultCacheManager
