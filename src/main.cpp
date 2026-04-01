#include <iostream>
#include <string>
#include <exception>
#include "LogAnalyzer.h"

// MPI is only linked when built with -DUSE_MPI
#ifdef USE_MPI
  #include <mpi.h>
#endif

// ─────────────────────────────────────────────
void printBanner() {
    std::cout << "========================================\n";
    std::cout << "  ParaLog - Parallel Log File Analyzer\n";
    std::cout << "  Version 2.0.0\n";
    std::cout << "========================================\n\n";
}

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " <log_file> [mode]\n\n";
    std::cout << "Modes:\n";
    std::cout << "  serial  - single-threaded (default baseline)\n";
    std::cout << "  openmp  - multi-threaded via OpenMP\n";
    std::cout << "  mpi     - distributed via MPI  (needs mpirun)\n";
    std::cout << "  gpu     - GPU accelerated       (not yet implemented)\n\n";
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

// ─────────────────────────────────────────────
//  main — only responsible for:
//    1. parse arguments
//    2. call the correct analyzer method
//    3. print results
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {

    // ── 1. Parse arguments ──────────────────
    if (argc < 2) {
        printBanner();
        printUsage(argv[0]);
        return 0;
    }

    const std::string filePath = argv[1];
    const std::string modeStr  = (argc > 2) ? argv[2] : "openmp";

    AnalysisMode mode;
    if      (modeStr == "serial") mode = AnalysisMode::SERIAL;
    else if (modeStr == "openmp") mode = AnalysisMode::PARALLEL_OMP;
    else if (modeStr == "mpi")    mode = AnalysisMode::PARALLEL_MPI;
    else if (modeStr == "gpu")    mode = AnalysisMode::PARALLEL_GPU;
    else {
        std::cerr << "[WARN] Unknown mode '" << modeStr
                  << "'. Defaulting to openmp.\n\n";
        mode = AnalysisMode::PARALLEL_OMP;
    }

    // ── 2. MPI needs Init/Finalize around the call ──
    //    Serial, OpenMP, GPU never touch MPI at all.
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

    // ── 3. Banner (rank 0 only for MPI) ─────
#ifdef USE_MPI
    int rank = 0;
    if (mode == AnalysisMode::PARALLEL_MPI)
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        printBanner();
        std::cout << "[INFO] File : " << filePath << "\n";
        std::cout << "[INFO] Mode : " << modeStr  << "\n\n";
        std::cout << std::flush; // Force flush for MPI
    }
#else
    printBanner();
    std::cout << "[INFO] File : " << filePath << "\n";
    std::cout << "[INFO] Mode : " << modeStr  << "\n\n";
#endif

    // ── 4. Dispatch — one call, no logic here ──
    try {
        LogAnalyzer analyzer;

        switch (mode) {
            case AnalysisMode::SERIAL:
                analyzer.analyzeSerial(filePath);
                break;
            case AnalysisMode::PARALLEL_OMP:
                analyzer.analyzeParallel(filePath);
                break;
            case AnalysisMode::PARALLEL_MPI:
                analyzer.analyzeDistributed(filePath);
                break;
            case AnalysisMode::PARALLEL_GPU:
                analyzer.analyzeGPU(filePath);
                break;
        }

        // ── 5. Print results (rank 0 only for MPI) ──
#ifdef USE_MPI
        if (mode != AnalysisMode::PARALLEL_MPI || rank == 0)
#endif
        {
            printResults(analyzer.getStatistics());
        }

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
#ifdef USE_MPI
        if (mode == AnalysisMode::PARALLEL_MPI)
            MPI_Abort(MPI_COMM_WORLD, 1);
#endif
        return 1;
    }

    // ── 6. MPI cleanup ──────────────────────
#ifdef USE_MPI
    if (mode == AnalysisMode::PARALLEL_MPI)
        MPI_Finalize();
#endif

    return 0;
}