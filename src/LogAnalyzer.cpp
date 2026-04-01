#include "LogAnalyzer.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>

void LogStatistics::print() const {
    std::cout << "\n========================================\n";
    std::cout << "  Log Analysis Results\n";
    std::cout << "========================================\n";
    std::cout << "Total Lines:    " << totalLines << "\n";
    std::cout << "ERROR count:    " << errorCount << "\n";
    std::cout << "WARNING count:  " << warningCount << "\n";
    std::cout << "INFO count:     " << infoCount << "\n";
    std::cout << "OTHER count:    " << otherCount << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Processing Time: " << processingTimeMs << " ms\n";
    std::cout << "========================================\n\n";
}

LogAnalyzer::LogAnalyzer() {
    // Constructor initializes the cumulative statistics
    m_stats = {};
}

LogAnalyzer::~LogAnalyzer() {
    // Destructor - placeholder for future cleanup
}

void LogAnalyzer::analyze(const std::vector<std::string>& lines, AnalysisMode mode) {
    auto startTime = std::chrono::high_resolution_clock::now();

    switch (mode) {
        case AnalysisMode::SERIAL:
            analyzeSerial(lines);
            break;
        case AnalysisMode::PARALLEL_OMP:
            analyzeParallel(lines);
            break;
        case AnalysisMode::DISTRIBUTED:
            analyzeDistributed(lines);
            break;
        case AnalysisMode::GPU_OPENCL:
            analyzeGPU(lines);
            break;
        default:
            analyzeSerial(lines);
            break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    m_st ats.processingTimeMs += duration.count();
}

LogStatistics LogAnalyzer::getStatistics() const {
    return m_stats;
}

void LogAnalyzer::analyzeSerial(const std::vector<std::string>& lines) {
    m_stats.totalLines += lines.size();
    
    for (const auto& line : lines) {
        std::string classification = classifyLine(line);
        
        if (classification == "ERROR") {
            m_stats.errorCount++;
        } else if (classification == "WARNING") {
            m_stats.warningCount++;
        } else if (classification == "INFO") {
            m_stats.infoCount++;
        } else {
            m_stats.otherCount++;
        }
    }
}

void LogAnalyzer::analyzeParallel(const std::vector<std::string>& lines) {
    size_t local_totalLines = 0;
    size_t local_errorCount = 0;
    size_t local_warningCount = 0;
    size_t local_infoCount = 0;
    size_t local_otherCount = 0;

    #pragma omp parallel for reduction(+:local_totalLines, local_errorCount, local_warningCount, local_infoCount, local_otherCount)
    for (size_t i = 0; i < lines.size(); ++i) {
        local_totalLines++;
        std::string classification = classifyLine(lines[i]);
        
        if (classification == "ERROR") {
            local_errorCount++;
        } else if (classification == "WARNING") {
            local_warningCount++;
        } else if (classification == "INFO") {
            local_infoCount++;
        } else {
            local_otherCount++;
        }
    }

    m_stats.totalLines += local_totalLines;
    m_stats.errorCount += local_errorCount;
    m_stats.warningCount += local_warningCount;
    m_stats.infoCount += local_infoCount;
    m_stats.otherCount += local_otherCount;
}

void LogAnalyzer::analyzeDistributed(const std::vector<std::string>& lines) {
    // Placeholder - will be implemented in future phases with MPI
    std::cout << "[WARN] Distributed MPI analysis not yet implemented\n";
    std::cout << "[INFO] Falling back to serial analysis\n";
    analyzeSerial(lines);
}

void LogAnalyzer::analyzeGPU(const std::vector<std::string>& lines) {
    
    std::cout << "[WARN] GPU OpenCL analysis not yet implemented\n";
    std::cout << "[INFO] Falling back to serial analysis\n";
    analyzeSerial(lines);
}

std::string LogAnalyzer::classifyLine(const std::string& line) const {
    // Simple classification based on keyword search
    // Future phases may implement more sophisticated parsing
    
    std::string upperLine = line;
    std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);
    
    if (upperLine.find("ERROR") != std::string::npos) {
        return "ERROR";
    } else if (upperLine.find("WARNING") != std::string::npos) {
        return "WARNING";
    } else if (upperLine.find("INFO") != std::string::npos) {
        return "INFO";
    }
    
    return "OTHER";
}
