#ifndef LOGREADER_H
#define LOGREADER_H

#include <string>
#include <vector>
#include <fstream>

/**
 * @brief LogReader class responsible for reading and parsing log files
 * 
 * This class will handle:
 * - Reading large log files efficiently in chunks
 * - Parsing log entries line by line
 * - Buffering data for processing
 * - Supporting different log formats (if needed)
 */
class LogReader {
public:
    /**
     * @brief Constructor that takes the file path and opens the file.
     * @param filePath Path to the log file
     */
    explicit LogReader(const std::string& filePath);
    
    /**
     * @brief Destructor
     */
    ~LogReader();

    /**
     * @brief Checks if the file was successfully opened.
     * @return True if the file is open, false otherwise.
     */
    bool isOpen() const;

    /**
     * @brief Reads the next chunk of lines from the file.
     * @param lines A vector to be filled with the lines from the chunk.
     * @param chunkSize The desired size of the chunk in bytes.
     * @return True if lines were read, false if the end of the file has been reached.
     */
    bool readNextChunk(std::vector<std::string>& lines, size_t chunkSize = 16 * 1024 * 1024);

private:
    std::ifstream m_fileStream;
    std::string m_leftover; // Holds partial line from the end of the previous chunk
};

#endif // LOGREADER_H
