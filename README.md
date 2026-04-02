# ParaLog - Parallel Log File Analyzer

A high-performance C++ application for analyzing large system and web server log files using multiple parallel computing paradigms.

## 📋 Project Overview

ParaLog demonstrates four different implementations of log file analysis:
- **Serial (Baseline)**: Traditional sequential processing
- **Parallel CPU**: Multi-threaded analysis using OpenMP
- **Distributed**: Cluster-based processing using MPI
- **GPU Accelerated**: High-performance computing using OpenCL

## 🎯 Features

- Analyze large log files efficiently
- Count ERROR, WARNING, and INFO messages
- Compare performance across different implementation strategies
- Generate statistical reports and benchmarks
- Modular architecture for easy extension

## 📁 Project Structure

```
ParaLog/
├── src/                    # Source implementation files
│   ├── main.cpp           # Main application entry point
│   ├── LogReader.cpp      # Log file reading module
│   ├── LogAnalyzer.cpp    # Analysis logic module
│   └── ResultCache.cpp    # Caching logic for comparison results
├── include/               # Header files
│   ├── LogReader.h        # Log reader interface
│   ├── LogAnalyzer.h      # Analyzer interface and statistics
│   └── ResultCache.h      # Cache management interface
├── data/                  # Sample log files
├── build/                 # Build output directory
│   └── comparison_cache.json # Cached analysis results
├── CMakeLists.txt         # CMake build configuration
├── build.ps1              # PowerShell build script
├── run_guide.md           # Quick command reference
└── README.md              # Project documentation
```

## 🛠️ Building and Running

This project uses a PowerShell script to simplify the build process on Windows. The workflow is designed around running individual analyses and then comparing the cached results.

### 1. Build the Project

Open a PowerShell terminal in the project root and run the `build.ps1` script.

```powershell
.\build.ps1
```

### 2. Run and Cache Analysis Modes

To perform a comparison, you must first run each analysis mode on the desired log file. This executes the analysis and saves the result to `build/comparison_cache.json`.

#### Serial Mode (Baseline)
```powershell
.\build\bin\paralog.exe data\large_sample.log serial
```

#### OpenMP Mode
```powershell
.\build\bin\paralog.exe data\large_sample.log openmp
```

#### MPI Mode
This requires an MPI environment (like MS-MPI).
```powershell
mpiexec -n 4 .\build\bin\paralog.exe data\large_sample.log mpi
```

### 3. Compare Cached Results

Once results are cached, use the `compare` command to generate a report with performance and speedup calculations.

```powershell
# SYNTAX: .\build\bin\paralog.exe compare <log_file_path>

.\build\bin\paralog.exe compare data\large_sample.log
```

## 🛠️ Build Requirements

- **C++ Compiler**: C++17 or later (MSVC, GCC, or Clang)
- **CMake**: Version 3.15 or higher
- **OpenMP**: For parallel CPU implementation (optional)
- **MPI**: For distributed implementation (optional, future phase)
- **OpenCL**: For GPU implementation (optional, future phase)

## 🚀 Building the Project

### Windows (Visual Studio)

```bash
# Create build directory
mkdir build
cd build

# Generate Visual Studio solution
cmake ..

# Build the project
cmake --build . --config Release

# Run the executable
.\bin\Release\paralog.exe
```

### Windows (MinGW)

```bash
# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake -G "MinGW Makefiles" ..

# Build the project
cmake --build .

# Run the executable
.\bin\paralog.exe
```

### Linux/macOS

```bash
# Create build directory
mkdir build && cd build

# Generate Makefiles
cmake ..

# Build the project
make

# Run the executable
./bin/paralog
```

## 📊 Usage

The primary workflow is to run analyses to cache results and then compare them.

**Run an analysis:**
```powershell
# .\build\bin\paralog.exe <log_file> [serial | openmp | mpi]
.\build\bin\paralog.exe data\sample.log serial
```

**Compare results for a file:**
```powershell
# .\build\bin\paralog.exe compare <log_file>
.\build\bin\paralog.exe compare data\sample.log
```

## 🧪 Testing with Sample Data

```bash
# Run individual analyses first, then compare.
.\build\bin\paralog.exe compare data\sample.log
.\build\bin\paralog.exe compare data\large_sample.log
.\build\bin\paralog.exe compare data\stress_test.log
```

## 📈 Development Phases

### ✅ Phase 1: Project Initialization (Complete)
- [x] Repository structure
- [x] CMake configuration

### ✅ Phase 2: Log File Reader (Complete)
- [x] `LogReader` class implementation
- [x] Error handling and sample data

### ✅ Phase 3: Serial Analyzer - Baseline (Complete)
- [x] Serial analysis algorithm
- [x] Statistics computation and performance measurement

### ✅ Phase 4: Parallel CPU (OpenMP) (Complete)
- [x] OpenMP parallel analysis implementation
- [x] Performance comparison vs. serial

### ✅ Phase 5: Distributed Processing (MPI) (Complete)
- [x] MPI implementation for distributed analysis
- [x] Data distribution strategy

### ✅ Phase 6: Cache-based Comparison (Complete)
- [x] Cache results from analysis runs into a JSON file
- [x] New `compare` mode to generate reports from cached data
- [x] Speedup calculations for all parallel modes vs. serial

### 🔄 Phase 7: GPU Acceleration (OpenCL) (Upcoming)
- [ ] OpenCL kernel implementation
- [ ] GPU memory management
- [ ] Performance optimization and comparison

## 🤝 Contributing

This is an educational project demonstrating parallel computing paradigms. Future enhancements may include:
- Support for different log formats
- Advanced pattern matching
- Real-time log streaming
- Web-based visualization dashboard

## 📄 License

This project is for educational purposes.

## 👨‍💻 Author

PDC Labs - Parallel and Distributed Computing

---

**Current Status**: Phase 3 Complete - Serial baseline analyzer implemented and tested!  
**Next up**: Phase 4 - OpenMP parallel implementation

