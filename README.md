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
│   └── LogAnalyzer.cpp    # Analysis logic module
├── include/               # Header files
│   ├── LogReader.h        # Log reader interface
│   └── LogAnalyzer.h      # Analyzer interface and statistics
├── data/                  # Sample log files
│   └── sample.log         # Example log file for testing
├── benchmarks/            # Performance comparison results
├── CMakeLists.txt         # CMake build configuration
└── README.md              # Project documentation
```

## � Building and Running

This project uses a PowerShell script to simplify the build and run process on Windows.

### 1. Build the Project

Open a PowerShell terminal in the project root and run the `build.ps1` script. You can specify `release` or `debug`.

```powershell
# Build in Release mode (recommended for performance)
.\build.ps1 release

# Build in Debug mode
.\build.ps1 debug
```

### 2. Run the Application

You can run the application using the `build.ps1` script or by calling the executable directly.

#### Using the Build Script

The script will automatically build the project if needed and then run it.

```powershell
# Run analysis in parallel mode (default)
.\build.ps1 run data/stress_test.log

# Explicitly run analysis in parallel mode
.\build.ps1 run data/stress_test.log parallel

# Explicitly run analysis in serial mode
.\build.ps1 run data/stress_test.log serial
```

#### Running the Executable Directly

Once the project is built, you can run the executable from the root directory.

```powershell
# SYNTAX: .\build\bin\paralog.exe <log_file_path> [mode]

# Run analysis in parallel mode (default)
.\build\bin\paralog.exe data/stress_test.log

# Explicitly run analysis in parallel mode
.\build\bin\paralog.exe data/stress_test.log parallel

# Explicitly run analysis in serial mode
.\build\bin\paralog.exe data/stress_test.log serial
```

## �🛠️ Build Requirements

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

```bash
# Run without arguments (shows help)
paralog

# Analyze a log file (basic usage)
paralog path/to/logfile.log

# Future: Specify analysis mode
paralog path/to/logfile.log --mode serial
paralog path/to/logfile.log --mode parallel
paralog path/to/logfile.log --mode distributed
paralog path/to/logfile.log --mode gpu
```

## 🧪 Testing with Sample Data

```bash
# Small sample (25 lines) - Quick test
paralog data/sample.log

# Medium sample (95 lines) - Detailed test
paralog data/large_sample.log

# Large stress test (25,000 lines) - Performance benchmark
paralog data/stress_test.log
```

## 📈 Development Phases

### ✅ Phase 1: Project Initialization (Complete)
- [x] Repository structure
- [x] CMake configuration
- [x] Placeholder modules
- [x] Basic compilation test

### ✅ Phase 2: Log File Reader (Complete)
- [x] LogReader class implementation
- [x] Line-by-line file reading
- [x] Error handling
- [x] Sample data files

### ✅ Phase 3: Serial Analyzer - Baseline (Complete)
- [x] Serial analysis algorithm
- [x] Pattern matching (ERROR/WARNING/INFO)
- [x] Statistics computation
- [x] Performance measurement with chrono
- [x] Throughput calculation
- [x] Tested with 25K lines: ~926K lines/second

### 🔄 Phase 4: Parallel CPU (OpenMP) (Upcoming)
- [ ] OpenMP parallel analysis
- [ ] Thread optimization
- [ ] Performance comparison vs serial
- [ ] Speedup calculation

### 🔄 Phase 5: Distributed Processing (MPI) (Upcoming)
- [ ] MPI implementation
- [ ] Data distribution strategy
- [ ] Cluster benchmarking
- [ ] Scalability testing

### 🔄 Phase 6: GPU Acceleration (OpenCL) (Upcoming)
- [ ] OpenCL kernel implementation
- [ ] GPU memory management
- [ ] Performance optimization
- [ ] Comparison with CPU implementations

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

