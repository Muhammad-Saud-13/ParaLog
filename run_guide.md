# ParaLog Run Guide

This guide provides the commands to build, run, and compare analyses with ParaLog.

## 1. Build the Project

Use the PowerShell build script to compile the project.

```powershell
.\build.ps1
```

## 2. Run and Cache Analysis Modes

To get a full comparison, you must first run each analysis mode individually. This will execute the analysis and save the result to a cache file (`build/comparison_cache.json`).

### Serial Mode

```powershell
.\build\bin\paralog.exe data\large_sample.log serial
```

### OpenMP Mode

```powershell
.\build\bin\paralog.exe data\large_sample.log openmp
```

### MPI Mode

This requires an MPI execution environment (like MS-MPI or Open MPI).

```powershell
mpiexec -n 4 .\build\bin\paralog.exe data\large_sample.log mpi
```

## 3. Compare Cached Results

Once you have run the analyses for a specific log file, you can generate a comparison report. This command reads the cached results and displays a summary with speedup calculations.

```powershell
.\build\bin\paralog.exe compare data\large_sample.log
```
