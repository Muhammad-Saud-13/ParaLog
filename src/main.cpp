#include <iostream>
#include <string>
#include <vector>
#include "LogReader.h"
#include "LogAnalyzer.h"

void printBanner() {
    std::cout << "========================================\n";
    std::cout << "  ParaLog - Parallel Log File Analyzer\n";
    std::cout << "  Version 1.0.0\n";
    std::cout << "========================================\n\n";
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <log_file_path>\n";
    std::cout << "\nAnalysis modes:\n";
    std::cout << "  ✓ Serial (baseline) - Implemented\n";
    std::cout << "  - Parallel CPU (OpenMP) - Coming soon\n";
    std::cout << "  - Distributed (MPI) - Coming soon\n";
    std::cout << "  - GPU Accelerated (OpenCL) - Coming soon\n";
}

int main(int argc, char* argv[]) {
    printBanner();
    
    std::cout << "[INFO] ParaLog is starting up...\n";
    std::cout << "[INFO] Environment configured successfully.\n";
    std::cout << "[INFO] System ready for log analysis.\n\n";
    
    if (argc < 2) {
        std::cout << "[WARN] No log file specified.\n\n";
        printUsage(argv[0]);
        return 0;
    }
    
    std::string logFilePath = argv[1];
    
    // Phase 3: Serial Baseline Implementation
    std::cout << "[INFO] Starting Phase 3: Serial Log Analysis\n";
    std::cout << "[INFO] Target file: " << logFilePath << "\n\n";
    
    // Step 1: Load log file using LogReader
    LogReader reader;
    std::cout << "[INFO] Loading log file...\n";
    
    if (!reader.loadFile(logFilePath)) {
        std::cerr << "[ERROR] Failed to load log file. Exiting.\n";
        return 1;
    }
    
    std::cout << "[INFO] Reading log lines into memory...\n";
    std::vector<std::string> logLines = reader.readAllLines();
    
    if (logLines.empty()) {
        std::cerr << "[ERROR] No data read from file. Exiting.\n";
        return 1;
    }
    
    std::cout << "[INFO] Successfully loaded " << logLines.size() << " lines\n\n";
    
    // Step 2: Analyze using serial baseline algorithm
    LogAnalyzer analyzer;
    std::cout << "[INFO] Running serial analysis (baseline)...\n";
    
    LogStatistics results = analyzer.analyzeSerial(logLines);
    
    // Step 3: Display results
    std::cout << "[INFO] Analysis complete!\n";
    results.print();
    
    // Performance summary
    double linesPerSecond = (results.processingTimeMs > 0) 
        ? (results.totalLines / (results.processingTimeMs / 1000.0)) 
        : 0;
    
    std::cout << "Performance Metrics:\n";
    std::cout << "  Throughput: " << static_cast<size_t>(linesPerSecond) << " lines/second\n";
    std::cout << "  Average: " << (results.processingTimeMs / static_cast<double>(results.totalLines)) 
              << " ms/line\n\n";
    
    std::cout << "[SUCCESS] Phase 3 complete - Serial baseline established!\n";
    
    return 0;
}
