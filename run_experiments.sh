#!/usr/bin/env bash
set -euo pipefail

# Automated compilation and safe execution script for Q4 - Virtual Memory & Demand Paging
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$ROOT_DIR/Source"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

echo "=========================================================================="
echo " STEP 1: COMPILING ALL SIX Q4 C PROGRAMS WITH GCC"
echo "=========================================================================="
gcc -std=c11 -Wall -Wextra "$SRC_DIR/page_fault_handling.c"      -o "$BUILD_DIR/page_fault_handling"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/effective_access_time.c"    -o "$BUILD_DIR/effective_access_time"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/memory_access_experiment.c" -o "$BUILD_DIR/memory_access_experiment"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/cow_fork_demo.c"            -o "$BUILD_DIR/cow_fork_demo"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/working_set_simulator.c"    -o "$BUILD_DIR/working_set_simulator"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/memory_growth_experiment.c" -o "$BUILD_DIR/memory_growth_experiment"
echo "All 6 programs compiled cleanly with zero errors."

echo -e "\n=========================================================================="
echo " STEP 2: PAGE-FAULT HANDLING DEMONSTRATION"
echo "=========================================================================="
"$BUILD_DIR/page_fault_handling"

echo -e "\n=========================================================================="
echo " STEP 3: EFFECTIVE ACCESS TIME & 10% SLOWDOWN ANALYSIS"
echo "=========================================================================="
"$BUILD_DIR/effective_access_time"

echo -e "\n=========================================================================="
echo " STEP 4: SEQUENTIAL vs. RANDOM MEMORY ACCESS EXPERIMENT"
echo "=========================================================================="
"$BUILD_DIR/memory_access_experiment"

echo -e "\n=========================================================================="
echo " STEP 5: COPY-ON-WRITE (CoW) WITH fork() DEMONSTRATION"
echo "=========================================================================="
"$BUILD_DIR/cow_fork_demo"

echo -e "\n=========================================================================="
echo " STEP 6: WORKING-SET MODEL, THRASHING & FRAME ALLOCATION SIMULATOR"
echo "=========================================================================="
"$BUILD_DIR/working_set_simulator"

echo -e "\n=========================================================================="
echo " STEP 7: CONTROLLED MEMORY-GROWTH EXPERIMENT (SAFE SIZES)"
echo "=========================================================================="
"$BUILD_DIR/memory_growth_experiment"

echo -e "\n=========================================================================="
echo " ALL Q4 EXPERIMENTS COMPLETED SAFELY"
echo "=========================================================================="
