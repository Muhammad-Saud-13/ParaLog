#include "ResultCache.h"
#include <fstream>
#include <iostream>
#include <sstream>

// Basic JSON serialization/deserialization.
// This is a minimal implementation to avoid adding a library dependency.

namespace ResultCacheManager {

ResultsCache load() {
    ResultsCache cache;
    std::ifstream file(CACHE_FILE_PATH);
    if (!file.is_open()) {
        return cache; // Return empty cache if file doesn't exist
    }

    std::string line;
    std::string currentFilePath;
    std::string currentMode;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string key;
        ss >> key;

        if (key == "\"log_file\":") {
            ss >> currentFilePath;
            currentFilePath = currentFilePath.substr(1, currentFilePath.length() - 3); // "path", -> path
        } else if (key == "\"mode\":") {
            ss >> currentMode;
            currentMode = currentMode.substr(1, currentMode.length() - 3); // "mode", -> mode
        } else if (key == "\"totalLines\":") {
            LogStatistics stats;
            stats.totalLines = std::stoull(line.substr(line.find(":") + 2));
            std::getline(file, line); stats.errorCount = std::stoull(line.substr(line.find(":") + 2));
            std::getline(file, line); stats.warningCount = std::stoull(line.substr(line.find(":") + 2));
            std::getline(file, line); stats.infoCount = std::stoull(line.substr(line.find(":") + 2));
            std::getline(file, line); stats.otherCount = std::stoull(line.substr(line.find(":") + 2));
            std::getline(file, line); stats.processingTimeMs = std::stod(line.substr(line.find(":") + 2));
            cache[currentFilePath][currentMode] = stats;
        }
    }
    return cache;
}

void save(const ResultsCache& cache) {
    std::ofstream file(CACHE_FILE_PATH);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Could not open cache file for writing: " << CACHE_FILE_PATH << std::endl;
        return;
    }

    file << "{\n";
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        file << "  \"" << it->first << "\": {\n";
        const auto& modeResults = it->second;
        for (auto mode_it = modeResults.begin(); mode_it != modeResults.end(); ++mode_it) {
            const auto& stats = mode_it->second;
            file << "    \"" << mode_it->first << "\": {\n";
            file << "      \"log_file\": \"" << it->first << "\",\n";
            file << "      \"mode\": \"" << mode_it->first << "\",\n";
            file << "      \"totalLines\": " << stats.totalLines << ",\n";
            file << "      \"errorCount\": " << stats.errorCount << ",\n";
            file << "      \"warningCount\": " << stats.warningCount << ",\n";
            file << "      \"infoCount\": " << stats.infoCount << ",\n";
            file << "      \"otherCount\": " << stats.otherCount << ",\n";
            file << "      \"processingTimeMs\": " << stats.processingTimeMs << "\n";
            file << "    }" << (std::next(mode_it) == modeResults.end() ? "" : ",") << "\n";
        }
        file << "  }" << (std::next(it) == cache.end() ? "" : ",") << "\n";
    }
    file << "}\n";
}

void updateAndSave(const std::string& logFilePath, const std::string& mode, const LogStatistics& stats) {
    ResultsCache cache = load();
    cache[logFilePath][mode] = stats;
    save(cache);
}

} // namespace ResultCacheManager
