#pragma once

#include "AppState.h"
#include <string>
#include <vector>

// ─── ResultsParser ────────────────────────────────────────────────────────────
// Reads comparison_cache.json and extracts results for a given log file.
// ─────────────────────────────────────────────────────────────────────────────
namespace ResultsParser {

    // Returns results for the given logFilePath.
    // outError is set on failure (file not found, parse error, key not found).
    // Returns empty vector + sets outError if anything goes wrong.
    std::vector<CompareResult> load(
        const std::string& cacheFilePath,
        const std::string& logFilePath,
        std::string&       outError
    );

} // namespace ResultsParser
