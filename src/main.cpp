#include <iostream>
#include <string>
#include <vector>
#include <exception> 
#include "LogReader.h"
#include <omp.h>
#include "LogReader.h"
#include "LogAnalyzer.h"

void printBanner() {
    std::cout << "========================================\n";
    std::cout << "  ParaLog - Parallel Log File Analyzer\n";
    std::cout << "  Version 1.0.0\n";
    std::cout << "========================================\n\n";
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <log_file_path> [mode]\n";
    std::cout << "\nAnalysis modes (optional, defaults to parallel):\n";
    std::cout << "  serial   - Single-threaded analysis (baseline)\n";
    std::cout << "  parallel - Multi-threaded analysis using OpenMP\n";
}

int main(int argc, char* argv[]) {
    try {
        printBanner();
        
        if (argc < 2) {
            printUsage(argv[0]);
            return 0;
        }
        
        std::string logFilePath = argv[1];
        AnalysisMode mode = AnalysisMode::PARALLEL_OMP; // Default to parallel
        std::string modeStr = "parallel";

        if (argc > 2) {
            modeStr = argv[2];
            if (modeStr == "serial") {
                mode = AnalysisMode::SERIAL;
            } else if (modeStr == "parallel") {
                mode = AnalysisMode::PARALLEL_OMP;
            } else {
                std::cout << "[WARN] Unknown mode '" << modeStr << "'. Defaulting to parallel.\n\n";
            }
        }
        
        std::cout << "[INFO] Starting analysis of: " << logFilePath << "\n";
        std::cout << "[INFO] Mode selected: " << modeStr << "\n\n";

        if (mode == AnalysisMode::PARALLEL_OMP) {
            std::cout << "[INFO] OpenMP will use up to " << omp_get_max_threads() << " threads.\n\n";
        }
        
        // Step 1: Initialize LogReader and LogAnalyzer
        LogReader reader(logFilePath);
        if (!reader.isOpen()) {
            std::cerr << "[ERROR] Failed to open log file. Exiting." << std::endl;
            return 1;
        }
        
        LogAnalyzer analyzer;
        std::vector<std::string> logLines;

        // Step 2: Read and analyze the file in chunks
        while (reader.readNextChunk(logLines)) {
            if (!logLines.empty()) {
                analyzer.analyze(logLines, mode);
            }
        }
        
        std::cout << "\n[INFO] Finished processing all chunks." << std::endl;

        // Step 3: Display results
        LogStatistics results = analyzer.getStatistics();
        
        if (results.totalLines == 0) {
            std::cout << "[WARN] No lines were processed. The log file might be empty." << std::endl;
        }

        std::cout << "[INFO] Analysis complete!" << std::endl;
        results.print();
        
        // Performance summary
        double linesPerSecond = (results.processingTimeMs > 0) 
            ? (static_cast<double>(results.totalLines) / (results.processingTimeMs / 1000.0)) 
            : 0;
        
        std::cout << "Performance Metrics:" << std::endl;
        std::cout << "  Throughput: " << static_cast<size_t>(linesPerSecond) << " lines/second" << std::endl;
        if (results.totalLines > 0) {
            std::cout << "  Average: " << (results.processingTimeMs / static_cast<double>(results.totalLines)) 
                      << " ms/line\n" << std::endl;
        }
        
        std::cout << "[SUCCESS] Analysis complete!\n" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n\n[FATAL ERROR] An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n\n[FATAL ERROR] An unknown non-standard exception occurred." << std::endl;
        return 1;
    }
    
    return 0;
}
