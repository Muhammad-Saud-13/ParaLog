#ifndef LOGREADER_H
#define LOGREADER_H

#include <string>
#include <vector>

/**
 * @brief LogReader class responsible for reading and parsing log files
 * 
 * This class will handle:
 * - Reading large log files efficiently
 * - Parsing log entries line by line
 * - Buffering data for processing
 * - Supporting different log formats (if needed)
 */
class LogReader {
public:
    /**
     * @brief Default constructor
     */
    LogReader();
    
    /**
     * @brief Destructor
     */
    ~LogReader();
    
    /**
     * @brief Load a log file for analysis
     * @param filePath Path to the log file
     * @return true if file loaded successfully, false otherwise
     */
    bool loadFile(const std::string& filePath);
    
    /**
     * @brief Get the total number of lines in the loaded file
     * @return Number of lines
     */
    size_t getLineCount() const;
    
    /**
     * @brief Read all lines from the log file
     * @return Vector containing all log lines
     */
    std::vector<std::string> readAllLines() const;
    
    /**
     * @brief Check if a file is currently loaded
     * @return true if file is loaded, false otherwise
     */
    bool isFileLoaded() const;
    
private:
    std::string m_filePath;
    bool m_fileLoaded;
    size_t m_lineCount;
};

#endif // LOGREADER_H
