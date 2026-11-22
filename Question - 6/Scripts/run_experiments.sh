#!/usr/bin/env bash
set -euo pipefail

# Master script for Q6 - Disk Management (Ubuntu WSL / GCC)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

mkdir -p build results

echo "=========================================================================="
echo " STEP 1: COMPILING Source/disk_management_analysis.c"
echo "=========================================================================="
gcc -std=c11 -Wall -Wextra -O2 Source/disk_management_analysis.c -lm -o build/disk_management_analysis
echo "Compiled cleanly: build/disk_management_analysis"

echo -e "\n=========================================================================="
echo " STEP 2: RUNNING RAID CAPACITY & FAILURE PROBABILITY ANALYSIS"
echo "=========================================================================="
./build/disk_management_analysis

echo -e "\n=========================================================================="
echo " STEP 3: COLLECTING LINUX DISK & SWAP INFORMATION (READ-ONLY)"
echo "=========================================================================="
bash scripts/disk_info.sh

echo -e "\n=========================================================================="
echo " STEP 4: RUNNING SAFE FILE READ-SPEED BENCHMARK"
echo "=========================================================================="
bash scripts/benchmark_read.sh

echo -e "\n=========================================================================="
echo " ALL Q6 EXPERIMENTS COMPLETED SAFELY"
echo "=========================================================================="
ls -lh results/
