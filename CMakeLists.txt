cmake_minimum_required(VERSION 3.15)
project(ParaLog VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/LogReader.cpp
    src/LogAnalyzer.cpp
)

# Header files
set(HEADERS
    include/LogReader.h
    include/LogAnalyzer.h
)

# Create executable
add_executable(paralog ${SOURCES} ${HEADERS})

# Compiler warnings
if(MSVC)
    target_compile_options(paralog PRIVATE /W4)
else()
    target_compile_options(paralog PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Find OpenMP for future parallel implementation
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(paralog PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found - parallel processing will be available")
else()
    message(WARNING "OpenMP not found - parallel processing will be disabled")
endif()

# Print configuration summary
message(STATUS "ParaLog Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Binary output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
