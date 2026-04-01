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
private:
    std::ifstream m_file;
    const size_t m_chunkSize;

public:
    /**
     * @brief Constructor that takes the file path and opens the file.
     * @param filePath Path to the log file
     * @param chunkSize The desired size of the chunk in bytes.
     */
    explicit LogReader(const std::string& filePath, size_t chunkSize = 8192);
    
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
     * @return True if lines were read, false if the end of the file has been reached.
     */
    bool readNextChunk(std::vector<std::string>& lines);

    /**
     * @brief Reads all lines from the file into a vector.
     * @param lines A vector to be filled with all lines from the file.
     */
    void readAllLines(std::vector<std::string>& lines);
};

#endif // LOGREADER_H
