#!/usr/bin/env bash
set -euo pipefail

# Master compilation and execution script for Q7 (Ubuntu WSL / GCC)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

mkdir -p build results

echo "=========================================================================="
echo " STEP 1: COMPILING ALL FOUR Q7 C PROGRAMS WITH GCC (-std=c11 -Wall -Wextra)"
echo "=========================================================================="
gcc -std=c11 -Wall -Wextra -O2 Source/memory_locking_test.c        -o build/memory_locking_test
gcc -std=c11 -Wall -Wextra -O2 Source/task_memory_simulator.c      -o build/task_memory_simulator
gcc -std=c11 -Wall -Wextra -O2 Source/page_fault_analysis.c        -o build/page_fault_analysis
gcc -std=c11 -Wall -Wextra -O2 Source/page_replacement_simulator.c -o build/page_replacement_simulator
echo "All 4 programs compiled cleanly with zero errors."

echo -e "\n=========================================================================="
echo " STEP 2: EXPERIMENT 1 - MEMORY LOCKING & REAL-TIME LOOP"
echo "=========================================================================="
./build/memory_locking_test | tee results/memory_locking_results.txt

echo -e "\n=========================================================================="
echo " STEP 3: EXPERIMENT 2 - 2 GB TASK MEMORY BUDGET SIMULATOR"
echo "=========================================================================="
./build/task_memory_simulator | tee results/task_memory_results.txt

echo -e "\n=========================================================================="
echo " STEP 4: EXPERIMENT 3 - PAGE-FAULT BEHAVIOUR ANALYSIS"
echo "=========================================================================="
./build/page_fault_analysis | tee results/page_fault_results.txt

echo -e "\n=========================================================================="
echo " STEP 5: EXPERIMENT 4 - THREE-TASK PAGE-REPLACEMENT SIMULATOR"
echo "=========================================================================="
./build/page_replacement_simulator | tee results/page_replacement_results.txt

echo -e "\n=========================================================================="
echo " ALL Q7 EXPERIMENTS COMPLETED SAFELY"
echo "=========================================================================="
ls -lh results/
