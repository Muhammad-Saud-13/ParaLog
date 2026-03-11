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
    GPU_OPENCL     // OpenCL GPU implementation
};

/**
 * @brief LogAnalyzer class responsible for analyzing log data
 * 
 * This class will support multiple analysis implementations:
 * - Serial baseline
 * - Parallel CPU (OpenMP)
 * - Distributed (MPI)
 * - GPU accelerated (OpenCL)
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
     * @brief Analyze log lines and compute statistics
     * @param lines Vector of log lines to analyze
     * @param mode Analysis mode to use
     * @return LogStatistics containing the results
     */
    LogStatistics analyze(const std::vector<std::string>& lines, 
                         AnalysisMode mode = AnalysisMode::SERIAL);
    
    /**
     * @brief Analyze log lines using serial implementation (baseline)
     * @param lines Vector of log lines to analyze
     * @return LogStatistics containing the results
     */
    LogStatistics analyzeSerial(const std::vector<std::string>& lines);
    
    /**
     * @brief Analyze log lines using OpenMP parallel implementation
     * @param lines Vector of log lines to analyze
     * @return LogStatistics containing the results
     */
    LogStatistics analyzeParallel(const std::vector<std::string>& lines);
    
    /**
     * @brief Analyze log lines using MPI distributed implementation
     * @param lines Vector of log lines to analyze
     * @return LogStatistics containing the results
     */
    LogStatistics analyzeDistributed(const std::vector<std::string>& lines);
    
    /**
     * @brief Analyze log lines using GPU OpenCL implementation
     * @param lines Vector of log lines to analyze
     * @return LogStatistics containing the results
     */
    LogStatistics analyzeGPU(const std::vector<std::string>& lines);
    
private:
    /**
     * @brief Classify a single log line
     * @param line The log line to classify
     * @return Classification result (ERROR, WARNING, INFO, OTHER)
     */
    std::string classifyLine(const std::string& line) const;
};

#endif // LOGANALYZER_H
