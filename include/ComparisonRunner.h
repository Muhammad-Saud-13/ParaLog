#pragma once

#include <string>
#include "LogAnalyzer.h"

struct ComparisonResult {
    LogStatistics serial;
    LogStatistics openmp;
    LogStatistics mpi;
};

ComparisonResult runAllModes(const std::string& filePath);
