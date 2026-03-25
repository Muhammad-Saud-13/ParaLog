#ifndef LOGANALYZER_H
#define LOGANALYZER_H

#include <string>
#include <vector>
#include <map>

/**
 * @brief Statistics structure to hold analysis results
 */
struct LogStatistics {
    size_t totalLines = 0;
    size_t errorCount = 0;
    size_t warningCount = 0;
    size_t infoCount = 0;
    size_t otherCount = 0;
    double processingTimeMs = 0.0;
    
    /**
     * @brief Print statistics to console
     */
    void print() const;
};

/**
 * @brief Analysis mode enumeration
 */
enum class AnalysisMode {
    SERIAL,        // Baseline serial implementation
    PARALLEL_OMP,  // OpenMP parallel implementation
    DISTRIBUTED,   // MPI distributed implementation
    GPU_OPENCL     // GPU-accelerated implementation
};

/**
 * @brief LogAnalyzer class responsible for analyzing log data
 */
class LogAnalyzer {
public:
    /**
     * @brief Default constructor
     */
    LogAnalyzer();
    
    /**
     * @brief Destructor
     */
    ~LogAnalyzer();
    
    /**
     * @brief Analyze a chunk of log lines using a specified mode
     * @param lines Vector of log lines to analyze
     * @param mode The analysis mode to use (SERIAL, PARALLEL_OMP, etc.)
     */
    void analyze(const std::vector<std::string>& lines, AnalysisMode mode);
    
    /**
     * @brief Get the final aggregated statistics
     * @return The final LogStatistics object
     */
    LogStatistics getStatistics() const;

private:
    /**
     * @brief Analyze a chunk of log lines using a serial algorithm
     * @param lines Vector of log lines to analyze
     */
    void analyzeSerial(const std::vector<std::string>& lines);
    
    /**
     * @brief Analyze a chunk of log lines using a parallel algorithm (placeholder)
     * @param lines Vector of log lines to analyze
     */
    void analyzeParallel(const std::vector<std::string>& lines);
    
    /**
     * @brief Analyze a chunk of log lines using a distributed algorithm (placeholder)
     * @param lines Vector of log lines to analyze
     */
    void analyzeDistributed(const std::vector<std::string>& lines);
    
    /**
     * @brief Analyze a chunk of log lines using a GPU-accelerated algorithm (placeholder)
     * @param lines Vector of log lines to analyze
     */
    void analyzeGPU(const std::vector<std::string>& lines);
    
    /**
     * @brief Classify a single log line
     * @param line The log line to classify
     * @return A string representing the classification (e.g., "ERROR")
     */
    std::string classifyLine(const std::string& line) const;
    
    LogStatistics m_stats; // Cumulative statistics
};

#endif // LOGANALYZER_H
