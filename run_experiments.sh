#!/usr/bin/env bash
set -euo pipefail

# Automated compilation and execution script for Q3 - Segmentation (Ubuntu WSL / GCC)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$ROOT_DIR/Source"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

echo "=========================================================================="
echo " STEP 1: COMPILING ALL Q3 C PROGRAMS WITH GCC"
echo "=========================================================================="
gcc -std=c11 -Wall -Wextra "$SRC_DIR/segmentation_translation.c" -o "$BUILD_DIR/segmentation_translation"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/segmented_paging.c"         -o "$BUILD_DIR/segmented_paging"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/segmentation_experiments.c" -o "$BUILD_DIR/segmentation_experiments"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/memory_maps.c"              -o "$BUILD_DIR/memory_maps"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/segfault_demo.c"            -o "$BUILD_DIR/segfault_demo"
echo "All 5 programs compiled cleanly with zero errors."

echo -e "\n=========================================================================="
echo " STEP 2: SEGMENT-TABLE ADDRESS TRANSLATION (8 TEST CASES)"
echo "=========================================================================="
"$BUILD_DIR/segmentation_translation"

echo -e "\n=========================================================================="
echo " STEP 3: SEGMENTATION WITH PAGING (EDUCATIONAL x86 MODEL)"
echo "=========================================================================="
"$BUILD_DIR/segmented_paging"

echo -e "\n=========================================================================="
echo " STEP 4: 20-SEGMENT EXTERNAL FRAGMENTATION & TABLE-SIZE COMPARISON"
echo "=========================================================================="
"$BUILD_DIR/segmentation_experiments"

echo -e "\n=========================================================================="
echo " STEP 5: LINUX PROCESS MEMORY SEGMENTS (/proc/<pid>/maps)"
echo "=========================================================================="
"$BUILD_DIR/memory_maps" --hold 1

echo -e "\n=========================================================================="
echo " STEP 6: SAFE SEGMENTATION FAULT DEMONSTRATION (fork + SIGSEGV)"
echo "=========================================================================="
"$BUILD_DIR/segfault_demo"

echo -e "\n=========================================================================="
echo " ALL Q3 EXPERIMENTS COMPLETED SUCCESSFULLY"
echo "=========================================================================="
