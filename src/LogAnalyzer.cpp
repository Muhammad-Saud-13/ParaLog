#include "LogAnalyzer.h"
#include "LogReader.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>
#include <cstring>
#include <regex>
#include <unordered_map>

#ifdef USE_MPI
  #include <mpi.h>
#endif

// ─────────────────────────────────────────────
void LogStatistics::print() const {
    std::cout << "\n========================================\n";
    std::cout << "  Log Analysis Results\n";
    std::cout << "========================================\n";
    std::cout << "Total Lines:     " << totalLines   << "\n";
    std::cout << "ERROR count:     " << errorCount   << "\n";
    std::cout << "WARNING count:   " << warningCount << "\n";
    std::cout << "INFO count:      " << infoCount    << "\n";
    std::cout << "OTHER count:     " << otherCount   << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Processing Time: " << processingTimeMs << " ms\n";
    std::cout << "========================================\n\n";
}

// ─────────────────────────────────────────────
LogAnalyzer::LogAnalyzer()  { m_stats = {}; }
LogAnalyzer::~LogAnalyzer() {}

LogStatistics LogAnalyzer::getStatistics() const { return m_stats; }

// ─────────────────────────────────────────────
// Regex-based classifier
// ─────────────────────────────────────────────
void LogAnalyzer::classifyLine(
    const std::string& line,
    size_t& errors, size_t& warnings, size_t& infos, size_t& others,
    size_t& timeouts, size_t& connRefused, size_t& loginFails,
    std::unordered_map<std::string, size_t>& localIpFreq
) const {
    static const std::regex errorRegex("ERROR|FAIL|CRITICAL", std::regex_constants::icase);
    static const std::regex warningRegex("WARNING|WARN", std::regex_constants::icase);
    static const std::regex infoRegex("INFO|DEBUG", std::regex_constants::icase);
    static const std::regex ipRegex(R"((\d{1,3}\.){3}\d{1,3})");

    if (std::regex_search(line, errorRegex)) {
        errors++;
    } else if (std::regex_search(line, warningRegex)) {
        warnings++;
    } else if (std::regex_search(line, infoRegex)) {
        infos++;
    } else {
        others++;
    }

    // Keyword search
    if (line.find("timeout") != std::string::npos) timeouts++;
    if (line.find("connection refused") != std::string::npos) connRefused++;
    if (line.find("failed login") != std::string::npos) loginFails++;

    // IP Address Extraction
    auto words_begin = std::sregex_iterator(line.begin(), line.end(), ipRegex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        localIpFreq[i->str()]++;
    }
}

// ─────────────────────────────────────────────
// SERIAL
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeSerial(const std::string& filePath) {
    LogReader reader(filePath);
    if (!reader.isOpen()) return;

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<std::string> lines;

    m_stats = {}; // Reset stats

    while (reader.readNextChunk(lines)) {
        for (const auto& line : lines) {
            m_stats.totalLines++;
            classifyLine(line, m_stats.errorCount, m_stats.warningCount, m_stats.infoCount, m_stats.otherCount,
                         timeoutCount, connectionRefusedCount, failedLoginCount,
                         ipFrequency);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    m_stats.processingTimeMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// ─────────────────────────────────────────────
// OPENMP
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeParallel(const std::string& filePath) {
    LogReader reader(filePath);
    if (!reader.isOpen()) return;

    std::cout << "[INFO] OpenMP using " << omp_get_max_threads() << " threads.\n";

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<std::string> lines;
    m_stats = {}; // Reset stats
    ipFrequency.clear();

    while (reader.readNextChunk(lines)) {
        if (lines.empty()) continue;

        size_t chunk_total = lines.size();
        
        #pragma omp parallel
        {
            std::unordered_map<std::string, size_t> local_ip_freq;
            size_t local_timeouts = 0;
            size_t local_conn_refused = 0;
            size_t local_login_fails = 0;
            size_t local_errors = 0, local_warnings = 0, local_infos = 0, local_others = 0;

            #pragma omp for schedule(dynamic, 64)
            for (size_t i = 0; i < lines.size(); ++i) {
                classifyLine(lines[i], local_errors, local_warnings, local_infos, local_others,
                             local_timeouts, local_conn_refused, local_login_fails,
                             local_ip_freq);
            }

            #pragma omp critical
            {
                timeoutCount += local_timeouts;
                connectionRefusedCount += local_conn_refused;
                failedLoginCount += local_login_fails;
                m_stats.errorCount += local_errors;
                m_stats.warningCount += local_warnings;
                m_stats.infoCount += local_infos;
                m_stats.otherCount += local_others;
                for (const auto& pair : local_ip_freq) {
                    ipFrequency[pair.first] += pair.second;
                }
            }
        }
        m_stats.totalLines += chunk_total;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    m_stats.processingTimeMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// ─────────────────────────────────────────────
// MPI (only loop enhanced)
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeDistributed(const std::string& filePath) {
#ifndef USE_MPI
    std::cerr << "[ERROR] Binary was built without MPI support. "
                 "Recompile with -DUSE_MPI.\n";
    return;
#else
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto t0 = MPI_Wtime();

    std::vector<std::string> local_lines;
    
    std::vector<char> send_buf;
    std::vector<int> send_counts(size);
    std::vector<int> displs(size);

    if (rank == 0) {
        LogReader reader(filePath);
        if (!reader.isOpen()) {
            std::cerr << "[ERROR] MPI: cannot open file: " << filePath << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
            return;
        }
        
        std::vector<std::string> all_lines;
        reader.readAllLines(all_lines);
        
        int lines_per_rank = all_lines.size() / size;
        int remainder = all_lines.size() % size;
        int current_line_idx = 0;

        for (int i = 0; i < size; ++i) {
            int num_lines_for_rank = lines_per_rank + (i < remainder ? 1 : 0);
            
            displs[i] = send_buf.size();
            
            for (int j = 0; j < num_lines_for_rank; ++j) {
                const std::string& line = all_lines[current_line_idx++];
                send_buf.insert(send_buf.end(), line.begin(), line.end());
                send_buf.push_back('\0'); // Null-terminate each string
            }
            send_counts[i] = send_buf.size() - displs[i];
        }
    }

    // Broadcast the sizes of the chunks to all processes
    MPI_Bcast(send_counts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs.data(), size, MPI_INT, 0, MPI_COMM_WORLD);

    // Prepare receive buffer on all processes
    std::vector<char> recv_buf(send_counts[rank]);

    // Scatter the data from rank 0 to all processes
    MPI_Scatterv(send_buf.data(), send_counts.data(), displs.data(), MPI_CHAR,
                 recv_buf.data(), send_counts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);
    
    // Unpack the received data into lines
    const char* ptr = recv_buf.data();
    const char* end = ptr + recv_buf.size();
    while (ptr < end) {
        const char* nul = (const char*)memchr(ptr, '\0', end - ptr);
        if (!nul) break;
        local_lines.emplace_back(ptr, nul - ptr);
        ptr = nul + 1;
    }

    size_t local_total = 0, local_error = 0, local_warning = 0, local_info = 0, local_other = 0;
    size_t local_timeouts = 0, local_conn_refused = 0, local_login_fails = 0;
    std::unordered_map<std::string, size_t> local_ip_freq;

    for (const auto& line : local_lines) {
        local_total++;
        classifyLine(line, local_error, local_warning, local_info, local_other,
                     local_timeouts, local_conn_refused, local_login_fails, local_ip_freq);
    }

    size_t global_total = 0, global_error = 0, global_warning = 0, global_info = 0, global_other = 0;
    size_t global_timeouts = 0, global_conn_refused = 0, global_login_fails = 0;

    MPI_Reduce(&local_total, &global_total, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_error, &global_error, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_warning, &global_warning, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_info, &global_info, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_other, &global_other, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_timeouts, &global_timeouts, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_conn_refused, &global_conn_refused, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_login_fails, &global_login_fails, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    auto t1 = MPI_Wtime();

    if (rank == 0) {
        m_stats.totalLines = global_total;
        m_stats.errorCount = global_error;
        m_stats.warningCount = global_warning;
        m_stats.infoCount = global_info;
        m_stats.otherCount = global_other;
        timeoutCount = global_timeouts;
        connectionRefusedCount = global_conn_refused;
        failedLoginCount = global_login_fails;
        ipFrequency = local_ip_freq; // Keep local map from rank 0
        m_stats.processingTimeMs = (t1 - t0) * 1000.0;
    }
#endif
}

// ─────────────────────────────────────────────
void LogAnalyzer::analyzeGPU(const std::string& filePath) {
    std::cout << "[WARN] GPU not implemented, using serial\n";
    analyzeSerial(filePath);
}