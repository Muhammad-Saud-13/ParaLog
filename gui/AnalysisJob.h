#pragma once

#include "AppState.h"
#include <string>
#include <thread>
#include <windows.h>

// ─── AnalysisJob ──────────────────────────────────────────────────────────────
// Launches paralog.exe (or mpiexec) as a child process, captures its
// stdout + stderr in real-time via anonymous pipes, and pushes every line
// into the provided JobState (thread-safe).
// ─────────────────────────────────────────────────────────────────────────────
class AnalysisJob {
public:
    AnalysisJob() = default;
    ~AnalysisJob() { cancel(); }

    // Non-copyable
    AnalysisJob(const AnalysisJob&)            = delete;
    AnalysisJob& operator=(const AnalysisJob&) = delete;

    // Launch helpers — exe is full path to paralog.exe
    void runSerial (JobState& job, const std::string& exe, const std::string& file);
    void runOpenMP (JobState& job, const std::string& exe, const std::string& file);
    void runMpi    (JobState& job, const std::string& mpiexec,
                    const std::string& exe, const std::string& file, int nprocs);

    // Terminate the running process (if any)
    void cancel();

    bool isRunning() const { return m_running; }

private:
    void launch(JobState& job, const std::string& commandLine);
    void readerThread(JobState& job, HANDLE hRead, HANDLE hProcess);

    HANDLE              m_hProcess{ INVALID_HANDLE_VALUE };
    std::thread         m_thread;
    std::atomic<bool>   m_running{ false };
};

// One global instance per mode — owned statically in main_gui.cpp
extern AnalysisJob g_jobSerial;
extern AnalysisJob g_jobOpenMP;
extern AnalysisJob g_jobMpi;
