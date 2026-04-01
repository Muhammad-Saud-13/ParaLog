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
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
}

bool LogReader::readNextChunk(std::vector<std::string>& lines) {
    lines.clear();

    if (!m_file.is_open() || m_file.eof()) {
        return false;
    }

    // Read a chunk of the file
    std::vector<char> buffer(m_chunkSize);
    m_file.read(buffer.data(), m_chunkSize);
    std::streamsize bytesRead = m_file.gcount();

    if (bytesRead == 0) {
        return false;
    }
    
    std::string chunkStr(buffer.data(), bytesRead);
    std::stringstream ss(chunkStr);
    std::string line;

    while(std::getline(ss, line)) {
        if(!line.empty()) {
            lines.push_back(line);
        }
    }

    return true;
}
