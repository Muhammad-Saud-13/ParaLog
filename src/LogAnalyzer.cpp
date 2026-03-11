#include "LogAnalyzer.h"
#include <iostream>
#include <algorithm>
#include <chrono>

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
    // Constructor - placeholder for future initialization
}

LogAnalyzer::~LogAnalyzer() {
    // Destructor - placeholder for future cleanup
}

LogStatistics LogAnalyzer::analyze(const std::vector<std::string>& lines, AnalysisMode mode) {
    switch (mode) {
        case AnalysisMode::SERIAL:
            return analyzeSerial(lines);
        case AnalysisMode::PARALLEL_OMP:
            return analyzeParallel(lines);
        case AnalysisMode::DISTRIBUTED:
            return analyzeDistributed(lines);
        case AnalysisMode::GPU_OPENCL:
            return analyzeGPU(lines);
        default:
            return analyzeSerial(lines);
    }
}

LogStatistics LogAnalyzer::analyzeSerial(const std::vector<std::string>& lines) {
    // Placeholder implementation with basic serial analysis
    LogStatistics stats;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    stats.totalLines = lines.size();
    
    for (const auto& line : lines) {
        std::string classification = classifyLine(line);
        
        if (classification == "ERROR") {
            stats.errorCount++;
        } else if (classification == "WARNING") {
            stats.warningCount++;
        } else if (classification == "INFO") {
            stats.infoCount++;
        } else {
            stats.otherCount++;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    stats.processingTimeMs = duration.count();
    
    return stats;
}

LogStatistics LogAnalyzer::analyzeParallel(const std::vector<std::string>& lines) {
    // Placeholder - will be implemented in future phases with OpenMP
    std::cout << "[WARN] Parallel OpenMP analysis not yet implemented\n";
    std::cout << "[INFO] Falling back to serial analysis\n";
    return analyzeSerial(lines);
}

LogStatistics LogAnalyzer::analyzeDistributed(const std::vector<std::string>& lines) {
    // Placeholder - will be implemented in future phases with MPI
    std::cout << "[WARN] Distributed MPI analysis not yet implemented\n";
    std::cout << "[INFO] Falling back to serial analysis\n";
    return analyzeSerial(lines);
}

LogStatistics LogAnalyzer::analyzeGPU(const std::vector<std::string>& lines) {
    // Placeholder - will be implemented in future phases with OpenCL
    std::cout << "[WARN] GPU OpenCL analysis not yet implemented\n";
    std::cout << "[INFO] Falling back to serial analysis\n";
    return analyzeSerial(lines);
}

std::string LogAnalyzer::classifyLine(const std::string& line) const {
    // Simple classification based on keyword search
    // Future phases may implement more sophisticated parsing
    
    std::string upperLine = line;
    std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);
    
    if (upperLine.find("ERROR") != std::string::npos) {
        return "ERROR";
    } else if (upperLine.find("WARNING") != std::string::npos || 
               upperLine.find("WARN") != std::string::npos) {
        return "WARNING";
    } else if (upperLine.find("INFO") != std::string::npos) {
        return "INFO";
    }
    
    return "OTHER";
}
