#include "LogReader.h"
#include <iostream>
#include <sstream>

LogReader::LogReader(const std::string& filePath, size_t chunkSize) : m_chunkSize(chunkSize) {
    m_file.open(filePath, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
        std::cerr << "[ERROR] LogReader: Failed to open file in constructor: " << filePath << std::endl << std::flush;
    }
}

LogReader::~LogReader() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

bool LogReader::isOpen() const {
    return m_file.is_open();
}

void LogReader::readAllLines(std::vector<std::string>& lines) {
    if (!isOpen()) return;
    m_file.clear();
    m_file.seekg(0, std::ios::beg);
    std::string line;
    while (std::getline(m_file, line)) {
        lines.push_back(line);
    }
}

bool LogReader::readNextChunk(std::vector<std::string>& lines) {
    lines.clear();

    if (!m_file.is_open() || m_file.eof()) {
        return false;
    }

    std::string line;
    for (size_t i = 0; i < m_chunkSize; ++i) {
        if (std::getline(m_file, line)) {
            lines.push_back(line);
        } else {
            break;
        }
    }

    return !lines.empty();
}
