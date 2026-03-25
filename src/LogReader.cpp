#include "LogReader.h"
#include <iostream>
#include <sstream>

LogReader::LogReader(const std::string& filePath) {
    m_fileStream.open(filePath, std::ios::in | std::ios::binary);
    if (!m_fileStream.is_open()) {
        std::cerr << "[ERROR] LogReader: Failed to open file in constructor: " << filePath << std::endl << std::flush;
    }
}

LogReader::~LogReader() {
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

bool LogReader::isOpen() const {
    return m_fileStream.is_open();
}

bool LogReader::readNextChunk(std::vector<std::string>& lines, size_t chunkSize) {
    lines.clear();

    if (!m_fileStream.is_open() || m_fileStream.eof()) {
        return false;
    }

    // Read a chunk of the file
    std::vector<char> buffer(chunkSize);
    m_fileStream.read(buffer.data(), chunkSize);
    std::streamsize bytesRead = m_fileStream.gcount();

    if (bytesRead == 0 && m_leftover.empty()) {
        return false;
    }

    // Combine leftover from previous chunk with the new buffer
    std::string data = m_leftover + std::string(buffer.data(), bytesRead);
    
    // Process the data into lines
    std::stringstream stream(data);
    std::string line;
    while (std::getline(stream, line)) {
        // Handle potential carriage returns for cross-platform compatibility
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    // If the file is not at the end and the last character wasn't a newline,
    // the last line in our chunk is partial. Save it for the next chunk.
    if (!m_fileStream.eof() && !data.empty() && data.back() != '\n') {
        m_leftover = lines.back();
        lines.pop_back();
    } else {
        m_leftover.clear();
    }

    // This ensures that even if the last chunk is empty, we don't stop prematurely
    // if there was leftover data to be processed.
    return !lines.empty() || !m_leftover.empty() || bytesRead > 0;
}
