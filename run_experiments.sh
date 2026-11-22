#!/usr/bin/env bash
set -euo pipefail

# Run from Q5_Source_Code root (or anywhere; automatically resolves root directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

mkdir -p build results

echo "=========================================================================="
echo " STEP 1: COMPILING ALL Q5 C PROGRAMS WITH GCC (-std=c11 -Wall -Wextra)"
echo "=========================================================================="
gcc -std=c11 -Wall -Wextra Source/complete_lru.c         -o build/complete_lru
gcc -std=c11 -Wall -Wextra Source/page_replacement.c     -o build/page_replacement
gcc -std=c11 -Wall -Wextra Source/clock_algorithm.c      -o build/clock_algorithm
gcc -std=c11 -Wall -Wextra Source/workload_generator.c   -o build/workload_generator
gcc -std=c11 -Wall -Wextra Source/performance_analysis.c -o build/performance_analysis
echo "All 5 C programs compiled cleanly."

echo -e "\n=========================================================================="
echo " STEP 2: RUNNING COMPLETED ASSIGNMENT PROGRAM (complete_lru)"
echo "=========================================================================="
./build/complete_lru

echo -e "\n=========================================================================="
echo " STEP 3: STEP-BY-STEP TRACES & BELADY'S ANOMALY (page_replacement)"
echo "=========================================================================="
./build/page_replacement

echo -e "\n=========================================================================="
echo " STEP 4: CLOCK (SECOND-CHANCE) ALGORITHM DEMO (clock_algorithm)"
echo "=========================================================================="
./build/clock_algorithm

echo -e "\n=========================================================================="
echo " STEP 5: STRONG-LOCALITY & RANDOM WORKLOADS (FRAMES 3 TO 10)"
echo "=========================================================================="
./build/workload_generator

echo -e "\n=========================================================================="
echo " STEP 6: 100,000-REFERENCE RUNTIME BENCHMARK (performance_analysis)"
echo "=========================================================================="
./build/performance_analysis

echo -e "\n=========================================================================="
echo " STEP 7: GENERATING PAGE-FAULT COMPARISON GRAPH"
echo "=========================================================================="
if command -v python3 >/dev/null 2>&1; then
    python3 scripts/plot_results.py
else
    echo "Warning: python3 not found; skipping graph regeneration."
fi

echo -e "\n=========================================================================="
echo " ALL Q5 EXPERIMENTS COMPLETED SUCCESSFULLY"
echo "=========================================================================="
