#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <regex>

// ─────────────────────────────────────────────
//  All 4 analysis modes
// ─────────────────────────────────────────────
enum class AnalysisMode {
    SERIAL        = 0,
    PARALLEL_OMP  = 1,
    PARALLEL_MPI  = 2,
    PARALLEL_GPU  = 3
};

// ─────────────────────────────────────────────
//  Result container
// ─────────────────────────────────────────────
struct LogStatistics {
    size_t totalLines   = 0;
    size_t errorCount   = 0;
    size_t warningCount = 0;
    size_t infoCount    = 0;
    size_t otherCount   = 0;
    double processingTimeMs = 0.0;

    void print() const;
};

// ─────────────────────────────────────────────
//  LogAnalyzer
// ─────────────────────────────────────────────
class LogAnalyzer {
private:
    LogStatistics m_stats;

    // Enhanced analysis members
    size_t timeoutCount;
    size_t connectionRefusedCount;
    size_t failedLoginCount;
    std::unordered_map<std::string, size_t> ipFrequency;

public:
    LogAnalyzer();
    ~LogAnalyzer();

    void analyzeSerial      (const std::string& filePath);
    void analyzeParallel    (const std::string& filePath);
    void analyzeDistributed (const std::string& filePath);
    void analyzeGPU         (const std::string& filePath);

    LogStatistics getStatistics() const;

private:
    void classifyLine(
        const std::string& line,
        size_t& errors, size_t& warnings, size_t& infos, size_t& others,
        size_t& timeouts, size_t& connRefused, size_t& loginFails,
        std::unordered_map<std::string, size_t>& localIpFreq
    ) const;
};