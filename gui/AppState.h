#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>

// ─── Compare result from cache ────────────────────────────────────────────────
struct CompareResult {
    std::string mode;
    size_t totalLines    = 0;
    size_t errorCount    = 0;
    size_t warningCount  = 0;
    size_t infoCount     = 0;
    size_t otherCount    = 0;
    double processingTimeMs = 0.0;
    double speedup       = 0.0;   // vs serial; 0 = N/A
};

// ─── Per-mode job state ───────────────────────────────────────────────────────
struct JobState {
    enum class Status { NotRun, Running, Done, Error } status{ Status::NotRun };
    int exitCode{ 0 };

    // Console output lines (ring-buffer: max 4000 lines)
    std::vector<std::string> lines;
    std::mutex               linesMutex;

    // Flag set by reader thread when the process finishes
    std::atomic<bool> finished{ false };
    std::atomic<bool> success{ false };

    void reset() {
        std::lock_guard<std::mutex> lk(linesMutex);
        lines.clear();
        status   = Status::NotRun;
        exitCode = 0;
        finished = false;
        success  = false;
    }
    void pushLine(const std::string& line) {
        std::lock_guard<std::mutex> lk(linesMutex);
        if (lines.size() >= 4000) lines.erase(lines.begin());
        lines.push_back(line);
    }
};

// ─── Central application state ────────────────────────────────────────────────
struct AppState {
    // File selection
    std::string selectedFile;

    // Analysis jobs
    JobState serial;
    JobState openmp;
    JobState mpi;

    // MPI configuration
    int         mpiProcesses{ 4 };
    char        mpiexecPath[512]{ "mpiexec" };

    // Which console tab is active ("serial" | "openmp" | "mpi")
    int activeConsoleTab{ 0 };

    // Compare panel
    std::vector<CompareResult> compareResults;
    bool  compareLoaded{ false };
    std::string compareError;

    // Helpers
    JobState& jobFor(int idx) {
        if (idx == 0) return serial;
        if (idx == 1) return openmp;
        return mpi;
    }
};
