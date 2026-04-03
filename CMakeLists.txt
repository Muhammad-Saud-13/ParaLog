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
    src/ComparisonRunner.cpp
    src/ResultCache.cpp
)

# Header files
set(HEADERS
    include/LogReader.h
    include/LogAnalyzer.h
    include/ComparisonRunner.h
    include/ResultCache.h
)

# Create executable
add_executable(paralog ${SOURCES} ${HEADERS})

# Compiler warnings + optimisation flags
if(MSVC)
    target_compile_options(paralog PRIVATE /W4)
    target_compile_options(paralog PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    # MinGW / GCC / Clang
    target_compile_options(paralog PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(paralog PRIVATE $<$<CONFIG:Release>:-O3>)
    target_compile_options(paralog PRIVATE $<$<CONFIG:Debug>:-g -O0>)
endif()

# ─────────────────────────────────────────────
#  OpenMP  (always ON — required for openmp mode)
# ─────────────────────────────────────────────
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(paralog PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found - parallel mode available")
else()
    message(FATAL_ERROR "OpenMP not found - required for parallel mode")
endif()

# ─────────────────────────────────────────────
#  MPI  (optional - OFF by default)
#
#  Your normal build commands stay exactly the same:
#    .\build\bin\paralog.exe data\large_sample.log serial
#    .\build\bin\paralog.exe data\large_sample.log openmp
#
#  To enable MPI, pass -DUSE_MPI=ON to cmake:
#    cmake .. -G "MinGW Makefiles" -DUSE_MPI=ON
# ─────────────────────────────────────────────
option(USE_MPI "Build with MPI support" ON)

if(USE_MPI)
    find_package(MPI REQUIRED)
    if(MPI_CXX_FOUND)
        target_compile_definitions(paralog PRIVATE USE_MPI)
        target_include_directories(paralog PRIVATE ${MPI_CXX_INCLUDE_DIRS})
        target_link_libraries(paralog PRIVATE ${MPI_CXX_LIBRARIES})
        message(STATUS "MPI found - distributed mode available")
    else()
        message(FATAL_ERROR "MPI not found but USE_MPI=ON was requested")
    endif()
else()
    message(STATUS "MPI disabled (build with -DUSE_MPI=ON to enable)")
endif()

# ─────────────────────────────────────────────
#  Configuration summary
# ─────────────────────────────────────────────
message(STATUS "ParaLog Configuration:")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:  ${CMAKE_CXX_STANDARD}")
message(STATUS "  Binary output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  MPI support:   ${USE_MPI}")

# ─────────────────────────────────────────────
#  GUI (Dear ImGui)
# ─────────────────────────────────────────────
option(BUILD_GUI "Build the ImGui GUI application" ON)
if(BUILD_GUI)
    add_subdirectory(gui)
endif()