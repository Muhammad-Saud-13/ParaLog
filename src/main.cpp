#include <iostream>
#include <string>
#include <exception>
#include "LogAnalyzer.h"
#include "ResultCache.h"

// MPI is only linked when built with -DUSE_MPI
#ifdef USE_MPI
  #include <mpi.h>
#endif

// ─────────────────────────────────────────────
void printBanner() {
    std::cout << "========================================\n";
    std::cout << "  ParaLog - Parallel Log File Analyzer\n";
    std::cout << "  Version 2.1.0 (Cache-based Comparison)\n";
    std::cout << "========================================\n\n";
}

void printUsage(const char* prog) {
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " <log_file> [serial | openmp | mpi]\n";
    std::cout << "  " << prog << " compare <log_file>\n\n";
    std::cout << "Description:\n";
    std::cout << "  Run an analysis in a specific mode (serial, openmp, mpi).\n";
    std::cout << "  The result is automatically cached.\n\n";
    std::cout << "  Use 'compare' to view the latest cached results for a log file.\n\n";
}

void printResults(const LogStatistics& s) {
    s.print();
    double throughput = (s.processingTimeMs > 0)
        ? (static_cast<double>(s.totalLines) / (s.processingTimeMs / 1000.0))
        : 0.0;
    std::cout << "Performance:\n";
    std::cout << "  Throughput : " << static_cast<size_t>(throughput) << " lines/sec\n";
    if (s.totalLines > 0) {
        std::cout << "  Avg/line   : "
                  << (s.processingTimeMs / static_cast<double>(s.totalLines))
                  << " ms\n";
    }
    std::cout << "\n[SUCCESS] Done.\n\n";
}

void runComparison(const std::string& logFilePath) {
    printBanner();
    std::cout << "Comparing results for: " << logFilePath << "\n\n";

    ResultsCache cache = ResultCacheManager::load();
    if (cache.find(logFilePath) == cache.end()) {
        std::cerr << "[ERROR] No cached results found for this file.\n";
        std::cerr << "Run analyses first: paralog " << logFilePath << " [serial|openmp|mpi]\n\n";
        return;
    }

    const auto& results = cache.at(logFilePath);
    const LogStatistics* serialStats = results.count("serial") ? &results.at("serial") : nullptr;
    const LogStatistics* openmpStats = results.count("openmp") ? &results.at("openmp") : nullptr;
    const LogStatistics* mpiStats = results.count("mpi") ? &results.at("mpi") : nullptr;

    std::cout << "----- SERIAL -----\n";
    if (serialStats) printResults(*serialStats);
    else std::cout << "No result cached. Run with 'serial' mode.\n\n";

    std::cout << "----- OPENMP -----\n";
    if (openmpStats) printResults(*openmpStats);
    else std::cout << "No result cached. Run with 'openmp' mode.\n\n";

    std::cout << "----- MPI -----\n";
    if (mpiStats) printResults(*mpiStats);
    else std::cout << "No result cached. Run with 'mpi' mode.\n\n";

    std::cout << "===== SPEEDUP =====\n";
    if (serialStats && openmpStats && openmpStats->processingTimeMs > 0) {
        double speedup = serialStats->processingTimeMs / openmpStats->processingTimeMs;
        std::cout << "OpenMP vs Serial: " << speedup << "x\n";
    }
    if (serialStats && mpiStats && mpiStats->processingTimeMs > 0) {
        double speedup = serialStats->processingTimeMs / mpiStats->processingTimeMs;
        std::cout << "MPI vs Serial   : " << speedup << "x\n";
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {

    if (argc < 2) {
        printBanner();
        printUsage(argv[0]);
        return 0;
    }

    // Handle 'compare' mode separately
    if (std::string(argv[1]) == "compare") {
        if (argc < 3) {
            std::cerr << "[ERROR] Missing log file for comparison.\n";
            printUsage(argv[0]);
            return 1;
        }
        runComparison(argv[2]);
        return 0;
    }

    // Standard analysis run
    const std::string filePath = argv[1];
    std::string modeStr  = (argc > 2) ? argv[2] : "openmp";

    AnalysisMode mode;
    if      (modeStr == "serial") mode = AnalysisMode::SERIAL;
    else if (modeStr == "openmp") mode = AnalysisMode::PARALLEL_OMP;
    else if (modeStr == "mpi")    mode = AnalysisMode::PARALLEL_MPI;
    else if (modeStr == "gpu")    mode = AnalysisMode::PARALLEL_GPU;
    else {
        std::cerr << "[WARN] Unknown mode '" << modeStr << "'. Defaulting to openmp.\n\n";
        mode = AnalysisMode::PARALLEL_OMP;
        modeStr = "openmp"; // Correct the mode string for caching
    }

#ifdef USE_MPI
    if (mode == AnalysisMode::PARALLEL_MPI) {
        MPI_Init(&argc, &argv);
    }
#else
    if (mode == AnalysisMode::PARALLEL_MPI) {
        std::cerr << "[ERROR] Binary not compiled with MPI support.\n"
                     "        Rebuild with -DUSE_MPI.\n";
        return 1;
    }
#endif

#ifdef USE_MPI
    int rank = 0;
    if (mode == AnalysisMode::PARALLEL_MPI)
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        printBanner();
        std::cout << "[INFO] File : " << filePath << "\n";
        std::cout << "[INFO] Mode : " << modeStr  << "\n\n";
        std::cout << std::flush;
    }
#else
    printBanner();
    std::cout << "[INFO] File : " << filePath << "\n";
    std::cout << "[INFO] Mode : " << modeStr  << "\n\n";
#endif

    try {
        LogAnalyzer analyzer;
        switch (mode) {
            case AnalysisMode::SERIAL:       analyzer.analyzeSerial(filePath);       break;
            case AnalysisMode::PARALLEL_OMP: analyzer.analyzeParallel(filePath);     break;
            case AnalysisMode::PARALLEL_MPI: analyzer.analyzeDistributed(filePath);  break;
            case AnalysisMode::PARALLEL_GPU: analyzer.analyzeGPU(filePath);          break;
        }

#ifdef USE_MPI
        if (mode != AnalysisMode::PARALLEL_MPI || rank == 0)
#endif
        {
            LogStatistics stats = analyzer.getStatistics();
            printResults(stats);
            // Only the main process (or non-MPI runs) should write to the cache.
            if (rank == 0) {
                ResultCacheManager::updateAndSave(filePath, modeStr, stats);
                std::cout << "[INFO] Results for '" << modeStr << "' mode have been cached.\n\n";
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
#ifdef USE_MPI
        if (mode == AnalysisMode::PARALLEL_MPI)
            MPI_Abort(MPI_COMM_WORLD, 1);
#endif
        return 1;
    }

#ifdef USE_MPI
    if (mode == AnalysisMode::PARALLEL_MPI)
        MPI_Finalize();
#endif

    return 0;
}