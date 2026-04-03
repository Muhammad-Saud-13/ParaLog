#include "AnalysisJob.h"
#include <sstream>
#include <stdexcept>

// Global instances
AnalysisJob g_jobSerial;
AnalysisJob g_jobOpenMP;
AnalysisJob g_jobMpi;

// ─────────────────────────────────────────────────────────────────────────────

void AnalysisJob::runSerial(JobState& job, const std::string& exe, const std::string& file) {
    launch(job, "\"" + exe + "\" \"" + file + "\" serial");
}

void AnalysisJob::runOpenMP(JobState& job, const std::string& exe, const std::string& file) {
    launch(job, "\"" + exe + "\" \"" + file + "\" openmp");
}

void AnalysisJob::runMpi(JobState& job, const std::string& mpiexecPath,
                         const std::string& exe, const std::string& file, int nprocs) {
    std::ostringstream cmd;
    cmd << "\"" << mpiexecPath << "\" -n " << nprocs
        << " \"" << exe << "\" \"" << file << "\" mpi";
    launch(job, cmd.str());
}

// ─────────────────────────────────────────────────────────────────────────────

void AnalysisJob::cancel() {
    if (m_hProcess != INVALID_HANDLE_VALUE) {
        TerminateProcess(m_hProcess, 1);
        CloseHandle(m_hProcess);
        m_hProcess = INVALID_HANDLE_VALUE;
    }
    if (m_thread.joinable()) m_thread.join();
    m_running = false;
}

// ─────────────────────────────────────────────────────────────────────────────

void AnalysisJob::launch(JobState& job, const std::string& commandLine) {
    if (m_running) cancel();

    job.reset();
    job.status = JobState::Status::Running;

    // Create anonymous pipes for stdout + stderr redirect
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    HANDLE hReadPipe  = INVALID_HANDLE_VALUE;
    HANDLE hWritePipe = INVALID_HANDLE_VALUE;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        job.pushLine("[GUI ERROR] Failed to create pipe for process I/O.");
        job.status  = JobState::Status::Error;
        job.success  = false;
        job.finished = true;
        return;
    }
    // Make write-end non-inheritable to child
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si{};
    si.cb          = sizeof(si);
    si.hStdOutput  = hWritePipe;
    si.hStdError   = hWritePipe;
    si.hStdInput   = INVALID_HANDLE_VALUE;
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};

    // Determine working directory: ParaLog root is ../.. from build/bin/paralog_gui.exe
    char exePathBuf[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exePathBuf, MAX_PATH);
    std::string exePath(exePathBuf);
    size_t pos = exePath.find_last_of("\\/");
    std::string workingDir = (pos != std::string::npos) ? exePath.substr(0, pos) + "\\..\\.." : ".";

    std::string cmdCopy = commandLine; // CreateProcessA needs mutable buffer
    BOOL ok = CreateProcessA(
        nullptr, cmdCopy.data(), nullptr, nullptr,
        TRUE,                          // inherit handles (pipe)
        CREATE_NO_WINDOW,
        nullptr, workingDir.c_str(),
        &si, &pi
    );

    // Close the write end in the parent — essential for ReadFile to EOF properly
    CloseHandle(hWritePipe);

    if (!ok) {
        CloseHandle(hReadPipe);
        std::ostringstream msg;
        msg << "[GUI ERROR] Failed to start process: " << commandLine
            << "\n           Windows error code: " << GetLastError()
            << "\n           Make sure paralog.exe is in the same folder as paralog_gui.exe.";
        job.pushLine(msg.str());
        job.status  = JobState::Status::Error;
        job.success  = false;
        job.finished = true;
        return;
    }

    m_hProcess = pi.hProcess;
    CloseHandle(pi.hThread);
    m_running  = true;

    // Spawn reader thread
    if (m_thread.joinable()) m_thread.join();
    m_thread = std::thread([this, &job, hReadPipe, hProcess = pi.hProcess]() {
        readerThread(job, hReadPipe, hProcess);
    });
}

// ─────────────────────────────────────────────────────────────────────────────

void AnalysisJob::readerThread(JobState& job, HANDLE hRead, HANDLE hProcess) {
    char  buf[4096];
    DWORD bytesRead = 0;
    std::string partial;           // accumulate partial lines across reads

    while (ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, nullptr) && bytesRead > 0) {
        buf[bytesRead] = '\0';
        partial += buf;

        // Split on newlines and push complete lines
        size_t pos = 0;
        size_t nl;
        while ((nl = partial.find('\n', pos)) != std::string::npos) {
            std::string line = partial.substr(pos, nl - pos);
            // Strip trailing \r
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) job.pushLine(line);
            pos = nl + 1;
        }
        partial = partial.substr(pos); // keep remainder
    }
    // Push any leftover partial line
    if (!partial.empty()) job.pushLine(partial);

    CloseHandle(hRead);

    // Get exit code
    DWORD exitCode = 0;
    GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);
    m_hProcess = INVALID_HANDLE_VALUE;

    job.exitCode = static_cast<int>(exitCode);
    job.success  = (exitCode == 0);
    job.status   = (exitCode == 0) ? JobState::Status::Done : JobState::Status::Error;
    job.finished = true;
    m_running    = false;
}
