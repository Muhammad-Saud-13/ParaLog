#include "LogAnalyzer.h"
#include "LogReader.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <omp.h>
#include <cstring>

#ifdef USE_MPI
  #include <mpi.h>
#endif

// ─────────────────────────────────────────────
//  LogStatistics::print
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
//  Constructor / Destructor
// ─────────────────────────────────────────────
LogAnalyzer::LogAnalyzer()  { m_stats = {}; }
LogAnalyzer::~LogAnalyzer() {}

LogStatistics LogAnalyzer::getStatistics() const { return m_stats; }

// ─────────────────────────────────────────────
//  classifyLine  (pure, thread-safe)
// ─────────────────────────────────────────────
std::string LogAnalyzer::classifyLine(const std::string& line) const {
    std::string upper = line;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper.find("ERROR")   != std::string::npos) return "ERROR";
    if (upper.find("WARNING") != std::string::npos) return "WARNING";
    if (upper.find("INFO")    != std::string::npos) return "INFO";
    return "OTHER";
}

// ─────────────────────────────────────────────
//  MODE 1 — Serial
//  Single thread. Reads file in chunks, classifies
//  each line one by one.
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeSerial(const std::string& filePath) {
    LogReader reader(filePath);
    if (!reader.isOpen()) {
        std::cerr << "[ERROR] Serial: cannot open file: " << filePath << "\n";
        return;
    }

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<std::string> lines;
    while (reader.readNextChunk(lines)) {
        for (const auto& line : lines) {
            m_stats.totalLines++;
            const std::string cls = classifyLine(line);
            if      (cls == "ERROR")   m_stats.errorCount++;
            else if (cls == "WARNING") m_stats.warningCount++;
            else if (cls == "INFO")    m_stats.infoCount++;
            else                       m_stats.otherCount++;
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    m_stats.processingTimeMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// ─────────────────────────────────────────────
//  MODE 2 — OpenMP
//  Reads each chunk, then classifies lines in
//  parallel using OMP reduction.
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeParallel(const std::string& filePath) {
    LogReader reader(filePath);
    if (!reader.isOpen()) {
        std::cerr << "[ERROR] OpenMP: cannot open file: " << filePath << "\n";
        return;
    }

    std::cout << "[INFO] OpenMP using " << omp_get_max_threads() << " threads.\n";

    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<std::string> lines;
    while (reader.readNextChunk(lines)) {
        if (lines.empty()) continue;

        size_t chunk_total   = 0;
        size_t chunk_error   = 0;
        size_t chunk_warning = 0;
        size_t chunk_info    = 0;
        size_t chunk_other   = 0;

        #pragma omp parallel for \
            reduction(+: chunk_total, chunk_error, chunk_warning, chunk_info, chunk_other) \
            schedule(dynamic, 64)
        for (size_t i = 0; i < lines.size(); ++i) {
            chunk_total++;
            const std::string cls = classifyLine(lines[i]);
            if      (cls == "ERROR")   chunk_error++;
            else if (cls == "WARNING") chunk_warning++;
            else if (cls == "INFO")    chunk_info++;
            else                       chunk_other++;
        }

        m_stats.totalLines   += chunk_total;
        m_stats.errorCount   += chunk_error;
        m_stats.warningCount += chunk_warning;
        m_stats.infoCount    += chunk_info;
        m_stats.otherCount   += chunk_other;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    m_stats.processingTimeMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// ─────────────────────────────────────────────
//  MODE 3 — MPI
//  Rank 0 reads the entire file and scatters
//  chunks to all ranks.  Every rank classifies
//  its own slice.  MPI_Reduce aggregates results
//  back to rank 0, which stores them in m_stats.
//
//  MPI_Init must be called before this method.
//  MPI_Finalize must be called after.
//  Both are handled in main.cpp.
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeDistributed(const std::string& filePath) {
#ifndef USE_MPI
    std::cerr << "[ERROR] Binary was built without MPI support. "
                 "Recompile with -DUSE_MPI.\n";
    return;
#else
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    auto t0 = std::chrono::high_resolution_clock::now();

    // ── Step 1: Rank 0 reads the file and sends each rank its slice ──
    std::vector<std::string> local_lines;

    if (world_rank == 0) {
        LogReader reader(filePath);
        if (!reader.isOpen()) {
            std::cerr << "[ERROR] MPI rank 0: cannot open file: " << filePath << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Read all lines into memory at rank 0
        std::vector<std::string> all_lines;
        std::vector<std::string> chunk;
        while (reader.readNextChunk(chunk)) {
            all_lines.insert(all_lines.end(), chunk.begin(), chunk.end());
        }

        int total      = static_cast<int>(all_lines.size());
        int base_chunk = total / world_size;
        int remainder  = total % world_size;
        int current    = 0;

        for (int rank = 0; rank < world_size; ++rank) {
            int count = base_chunk + (rank < remainder ? 1 : 0);

            if (rank == 0) {
                // Keep rank 0's slice locally
                local_lines.assign(
                    all_lines.begin() + current,
                    all_lines.begin() + current + count
                );
            } else {
                // Serialize: lines joined by '\0' delimiter
                std::string payload;
                payload.reserve(static_cast<size_t>(count) * 80);
                for (int j = 0; j < count; ++j) {
                    payload += all_lines[current + j];
                    payload += '\0';
                }
                MPI_Send(
                    payload.data(),
                    static_cast<int>(payload.size()),
                    MPI_CHAR, rank, 0, MPI_COMM_WORLD
                );
            }
            current += count;
        }

        std::cout << "[INFO] MPI rank 0 distributed " << total
                  << " lines across " << world_size << " ranks.\n";

    } else {
        // ── Worker ranks receive and deserialize their slice ──
        MPI_Status status;
        int count = 0;
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &count);

        std::vector<char> buffer(count);
        MPI_Recv(buffer.data(), count, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

        // Walk through null-delimited segments correctly
        // (cannot use std::string(ptr) — it stops at first '\0')
        const char* ptr = buffer.data();
        const char* end = ptr + count;
        while (ptr < end) {
            size_t len = static_cast<size_t>(end - ptr);
            // find next '\0'
            const char* nul = static_cast<const char*>(memchr(ptr, '\0', len));
            if (!nul) break;
            local_lines.emplace_back(ptr, nul - ptr);
            ptr = nul + 1;
        }
    }

    // ── Step 2: Every rank classifies its own slice (serial loop per rank) ──
    size_t local_total   = 0;
    size_t local_error   = 0;
    size_t local_warning = 0;
    size_t local_info    = 0;
    size_t local_other   = 0;

    for (const auto& line : local_lines) {
        local_total++;
        const std::string cls = classifyLine(line);
        if      (cls == "ERROR")   local_error++;
        else if (cls == "WARNING") local_warning++;
        else if (cls == "INFO")    local_info++;
        else                       local_other++;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double local_time = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // ── Step 3: MPI_Reduce — aggregate all ranks' counts to rank 0 ──
    size_t g_total   = 0, g_error = 0, g_warning = 0, g_info = 0, g_other = 0;
    double g_time    = 0.0;

    MPI_Reduce(&local_total,   &g_total,   1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_error,   &g_error,   1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_warning, &g_warning, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_info,    &g_info,    1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_other,   &g_other,   1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time,    &g_time,    1, MPI_DOUBLE,        MPI_MAX, 0, MPI_COMM_WORLD);

    // ── Step 4: Only rank 0 stores the final stats ──
    if (world_rank == 0) {
        m_stats.totalLines      = g_total;
        m_stats.errorCount      = g_error;
        m_stats.warningCount    = g_warning;
        m_stats.infoCount       = g_info;
        m_stats.otherCount      = g_other;
        m_stats.processingTimeMs = g_time;
    }
#endif
}

// ─────────────────────────────────────────────
//  MODE 4 — GPU
//  Placeholder. Falls back to serial for now.
//  Replace the body with CUDA kernel launch
//  when GPU support is implemented.
// ─────────────────────────────────────────────
void LogAnalyzer::analyzeGPU(const std::string& filePath) {
    std::cout << "[WARN] GPU analysis not yet implemented. "
                 "Falling back to serial.\n\n";
    analyzeSerial(filePath);
}