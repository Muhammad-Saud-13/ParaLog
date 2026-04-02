#include "LogAnalyzer.h"
#include "LogReader.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>
#include <cstring>
#include <regex>
#include <unordered_map>
#include <sstream>

#ifdef USE_MPI
  #include <mpi.h>

  // Custom MPI data type for LogStatistics
  static MPI_Datatype MPI_Line_Stats;
  static MPI_Op MPI_SUM_Stats;

  static void sumLogStatistics(void* invec, void* inoutvec, int* len, [[maybe_unused]] MPI_Datatype* datatype) {
    LogStatistics* in = (LogStatistics*)invec;
    LogStatistics* inout = (LogStatistics*)inoutvec;

    for (int i = 0; i < *len; i++) {
      inout[i].totalLines += in[i].totalLines;
      inout[i].errorCount += in[i].errorCount;
      inout[i].warningCount += in[i].warningCount;
      inout[i].infoCount += in[i].infoCount;
      inout[i].otherCount += in[i].otherCount;
    }
  }

  static void defineMpiStatsType() {
    static bool typeDefined = false;
    if (typeDefined)
      return;

    const int nitems = 5;
    int blocklengths[nitems] = {1, 1, 1, 1, 1};
    MPI_Datatype types[nitems] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG,
                                    MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG,
                                    MPI_UNSIGNED_LONG};

    MPI_Aint offsets[nitems];
    offsets[0] = offsetof(LogStatistics, totalLines);
    offsets[1] = offsetof(LogStatistics, errorCount);
    offsets[2] = offsetof(LogStatistics, warningCount);
    offsets[3] = offsetof(LogStatistics, infoCount);
    offsets[4] = offsetof(LogStatistics, otherCount);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_Line_Stats);
    MPI_Type_commit(&MPI_Line_Stats);

    MPI_Op_create(sumLogStatistics, 1, &MPI_SUM_Stats);

    typeDefined = true;
  }
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
LogAnalyzer::LogAnalyzer()
    : m_stats({}),
      timeoutCount(0),
      connectionRefusedCount(0),
      failedLoginCount(0),
      ipFrequency() {
}

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
// MPI - FINAL WORKING VERSION
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
    defineMpiStatsType();

    auto t0 = MPI_Wtime();

    std::vector<std::string> all_lines;
    
    // ============ STEP 1: Rank 0 reads file ============
    if (rank == 0) {
        LogReader reader(filePath);
        if (!reader.isOpen()) {
            std::cerr << "[ERROR] MPI: cannot open file: " << filePath << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
            return;
        }
        reader.readAllLines(all_lines);
    }

    // ============ STEP 2: Broadcast line count ============
    int total_lines = (rank == 0) ? (int)all_lines.size() : 0;
    MPI_Bcast(&total_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (total_lines <= 0) {
        if (rank == 0) {
            std::cerr << "[ERROR] No lines to process.\n";
        }
        return;
    }

    // ============ STEP 3: Rank 0 serializes ============
    std::string serialized;
    
    if (rank == 0) {
        for (const auto& line : all_lines) {
            serialized += line;
            serialized += '\n';
        }
    }

    // ============ STEP 4: Broadcast serialized size ============
    int serialized_size = (rank == 0) ? (int)serialized.size() : 0;
    MPI_Bcast(&serialized_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // ============ STEP 5: Allocate and broadcast serialized data ============
    if (rank != 0) {
        serialized.resize(serialized_size);
    }
    
    MPI_Bcast(const_cast<char*>(serialized.c_str()), serialized_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    // ============ STEP 6: All ranks parse the serialized string ============
    all_lines.clear();
    std::istringstream iss(serialized);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            all_lines.push_back(line);
        }
    }

    // ============ STEP 7: Distribute work ============
    int lines_per_rank = total_lines / size;
    int remainder = total_lines % size;
    int my_start = rank * lines_per_rank + std::min(rank, remainder);
    int my_count = lines_per_rank + (rank < remainder ? 1 : 0);

    // ============ STEP 8: Process local lines ============
    LogStatistics local_stats = {};
    size_t local_timeouts = 0, local_conn_refused = 0, local_login_fails = 0;
    std::unordered_map<std::string, size_t> local_ip_freq;

    for (int i = my_start; i < my_start + my_count; ++i) {
        if (i >= 0 && i < (int)all_lines.size()) {
            local_stats.totalLines++;
            classifyLine(all_lines[i], local_stats.errorCount, local_stats.warningCount,
                         local_stats.infoCount, local_stats.otherCount, local_timeouts,
                         local_conn_refused, local_login_fails, local_ip_freq);
        }
    }

    // ============ STEP 9: Reduce statistics using built-in MPI_SUM ============
    // ✅ KEY FIX: Use built-in MPI_SUM instead of custom operation!
    LogStatistics global_stats = {};
    
    // Reduce totalLines
    unsigned long temp_total = local_stats.totalLines;
    MPI_Reduce(&temp_total, (unsigned long*)&global_stats.totalLines, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);
    
    // Reduce errorCount
    temp_total = local_stats.errorCount;
    MPI_Reduce(&temp_total, (unsigned long*)&global_stats.errorCount, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);
    
    // Reduce warningCount
    temp_total = local_stats.warningCount;
    MPI_Reduce(&temp_total, (unsigned long*)&global_stats.warningCount, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);
    
    // Reduce infoCount
    temp_total = local_stats.infoCount;
    MPI_Reduce(&temp_total, (unsigned long*)&global_stats.infoCount, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);
    
    // Reduce otherCount
    temp_total = local_stats.otherCount;
    MPI_Reduce(&temp_total, (unsigned long*)&global_stats.otherCount, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
               MPI_COMM_WORLD);

    size_t global_timeouts = 0, global_conn_refused = 0, global_login_fails = 0;
    MPI_Reduce(&local_timeouts, &global_timeouts, 1, MPI_UNSIGNED_LONG, MPI_SUM,
               0, MPI_COMM_WORLD);
    MPI_Reduce(&local_conn_refused, &global_conn_refused, 1, MPI_UNSIGNED_LONG,
               MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_login_fails, &global_login_fails, 1, MPI_UNSIGNED_LONG,
               MPI_SUM, 0, MPI_COMM_WORLD);

    // ============ STEP 10: Gather IP frequency maps ============
    std::string local_ip_str;
    for (const auto& pair : local_ip_freq) {
        local_ip_str += pair.first + ":" + std::to_string(pair.second) + ";";
    }

    int local_size = local_ip_str.size();
    std::vector<int> all_sizes(size, 0);
    MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0,
               MPI_COMM_WORLD);

    std::vector<char> all_ip_strs_buf;
    std::vector<int> ip_displs(size, 0);
    if (rank == 0) {
        int total_ip_buf_size = 0;
        for (int i = 0; i < size; ++i) {
            ip_displs[i] = total_ip_buf_size;
            total_ip_buf_size += all_sizes[i];
        }
        all_ip_strs_buf.resize(total_ip_buf_size);
    }

    MPI_Gatherv(local_ip_str.data(), local_size, MPI_CHAR,
                all_ip_strs_buf.data(), all_sizes.data(), ip_displs.data(),
                MPI_CHAR, 0, MPI_COMM_WORLD);

    auto t1 = MPI_Wtime();

    // ============ STEP 11: Finalize on rank 0 ============
    if (rank == 0) {
        ipFrequency.clear();
        for (int i = 0; i < size; ++i) {
            if (all_sizes[i] > 0) {
                std::string s(all_ip_strs_buf.data() + ip_displs[i], all_sizes[i]);
                size_t start = 0;
                while (start < s.length()) {
                    size_t end_key = s.find(':', start);
                    size_t end_val = s.find(';', end_key);
                    if (end_key == std::string::npos || end_val == std::string::npos)
                        break;

                    std::string ip = s.substr(start, end_key - start);
                    size_t count =
                        std::stoul(s.substr(end_key + 1, end_val - (end_key + 1)));
                    ipFrequency[ip] += count;
                    start = end_val + 1;
                }
            }
        }

        m_stats = global_stats;
        timeoutCount = global_timeouts;
        connectionRefusedCount = global_conn_refused;
        failedLoginCount = global_login_fails;
        m_stats.processingTimeMs = (t1 - t0) * 1000.0;
    }
#endif
}

// ─────────────────────────────────────────────
void LogAnalyzer::analyzeGPU(const std::string& filePath) {
    std::cout << "[WARN] GPU not implemented, using serial\n";
    analyzeSerial(filePath);
}