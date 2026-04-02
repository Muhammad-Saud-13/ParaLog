#include "ComparisonRunner.h"
#include "LogAnalyzer.h"

ComparisonResult runAllModes(const std::string& filePath) {
    ComparisonResult result;

    // Serial analysis
    LogAnalyzer analyzerSerial;
    analyzerSerial.analyzeSerial(filePath);
    result.serial = analyzerSerial.getStatistics();

    // OpenMP analysis
    LogAnalyzer analyzerOpenMP;
    analyzerOpenMP.analyzeParallel(filePath);
    result.openmp = analyzerOpenMP.getStatistics();

    // MPI placeholder
    result.mpi.processingTimeMs = -1.0;
    result.mpi.totalLines = 0;
    result.mpi.errorCount = 0;
    result.mpi.warningCount = 0;
    result.mpi.infoCount = 0;
    result.mpi.otherCount = 0;

    return result;
}
