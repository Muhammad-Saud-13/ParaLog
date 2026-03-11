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
# Use the provided sample log file
paralog data/sample.log
```

## 📈 Development Phases

### ✅ Phase 1: Project Initialization (Current)
- [x] Repository structure
- [x] CMake configuration
- [x] Placeholder modules
- [x] Basic compilation test

### 🔄 Phase 2: Serial Implementation (Upcoming)
- [ ] Complete LogReader implementation
- [ ] Serial analysis algorithm
- [ ] Performance measurement
- [ ] Unit tests

### 🔄 Phase 3: Parallel CPU (OpenMP) (Upcoming)
- [ ] OpenMP parallel analysis
- [ ] Thread optimization
- [ ] Performance comparison

### 🔄 Phase 4: Distributed Processing (MPI) (Upcoming)
- [ ] MPI implementation
- [ ] Data distribution strategy
- [ ] Cluster benchmarking

### 🔄 Phase 5: GPU Acceleration (OpenCL) (Upcoming)
- [ ] OpenCL kernel implementation
- [ ] GPU memory management
- [ ] Performance optimization

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

**Current Status**: Phase 1 Complete - Project structure initialized and ready for implementation.
