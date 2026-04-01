#pragma once

#include <string>
#include <vector>

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
//
//  Each public method is fully self-contained:
//    - opens the file itself via LogReader
//    - runs its own parallelism strategy
//    - stores results in m_stats
//
//  main.cpp only calls one of these, then prints.
// ─────────────────────────────────────────────
class LogAnalyzer {
public:
    LogAnalyzer();
    ~LogAnalyzer();

    // Four fully self-contained entry points
    void analyzeSerial      (const std::string& filePath);
    void analyzeParallel    (const std::string& filePath);
    void analyzeDistributed (const std::string& filePath);
    void analyzeGPU         (const std::string& filePath);

    LogStatistics getStatistics() const;

private:
    LogStatistics m_stats;

    // Shared line classifier (pure function, thread-safe)
    std::string classifyLine(const std::string& line) const;
};