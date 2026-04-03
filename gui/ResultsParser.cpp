#include "ResultsParser.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace ResultsParser {

// Helper: try both the exact path and a "data\filename" fallback
static bool findFileEntry(const json& root, const std::string& logFilePath, json& outEntry) {
    // Exact match first
    if (root.contains(logFilePath)) {
        outEntry = root[logFilePath];
        return true;
    }

    // Try normalized versions: replace '\\' with '/' and vice-versa
    std::string normalized = logFilePath;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
    if (root.contains(normalized)) { outEntry = root[normalized]; return true; }

    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    if (root.contains(normalized)) { outEntry = root[normalized]; return true; }

    // Try basename match
    std::string basename = logFilePath;
    size_t slash = basename.find_last_of("/\\");
    if (slash != std::string::npos) basename = basename.substr(slash + 1);

    for (auto& [key, val] : root.items()) {
        std::string kb = key;
        size_t ks = kb.find_last_of("/\\");
        if (ks != std::string::npos) kb = kb.substr(ks + 1);
        if (kb == basename) { outEntry = val; return true; }
    }

    return false;
}

static CompareResult extractMode(const json& modeJson, const std::string& mode) {
    CompareResult r;
    r.mode           = mode;
    r.totalLines     = modeJson.value("totalLines",      size_t(0));
    r.errorCount     = modeJson.value("errorCount",      size_t(0));
    r.warningCount   = modeJson.value("warningCount",    size_t(0));
    r.infoCount      = modeJson.value("infoCount",       size_t(0));
    r.otherCount     = modeJson.value("otherCount",      size_t(0));
    r.processingTimeMs = modeJson.value("processingTimeMs", 0.0);
    return r;
}

std::vector<CompareResult> load(const std::string& cacheFilePath,
                                 const std::string& logFilePath,
                                 std::string&       outError) {
    outError.clear();

    // Open cache file
    std::ifstream f(cacheFilePath);
    if (!f.is_open()) {
        outError = "Cache file not found:\n  " + cacheFilePath +
                   "\nRun at least one analysis mode first.";
        return {};
    }

    // Parse JSON
    json root;
    try {
        f >> root;
    } catch (const json::exception& e) {
        outError = std::string("Failed to parse cache JSON: ") + e.what();
        return {};
    }

    // Find entry for this log file
    json entry;
    if (!findFileEntry(root, logFilePath, entry)) {
        outError = "No cached results for:\n  " + logFilePath +
                   "\n\nRun at least one analysis mode for this file first.";
        return {};
    }

    // Extract available modes in display order
    std::vector<CompareResult> results;
    const std::vector<std::string> modeOrder = { "serial", "openmp", "mpi" };
    for (const auto& mode : modeOrder) {
        if (entry.contains(mode)) {
            results.push_back(extractMode(entry[mode], mode));
        }
    }

    if (results.empty()) {
        outError = "Cache entry found but contains no mode results.";
        return {};
    }

    // Compute speedup relative to serial
    double serialTime = 0.0;
    for (const auto& r : results) {
        if (r.mode == "serial") { serialTime = r.processingTimeMs; break; }
    }
    for (auto& r : results) {
        if (r.mode != "serial" && serialTime > 0.0 && r.processingTimeMs > 0.0)
            r.speedup = serialTime / r.processingTimeMs;
    }

    return results;
}

} // namespace ResultsParser
