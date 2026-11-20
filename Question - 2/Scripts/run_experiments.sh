#!/usr/bin/env bash
set -euo pipefail

# Automated compilation and execution script for Q2 - Paging (Ubuntu WSL / GCC)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$ROOT_DIR/Source"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

echo "============================================================"
echo " STEP 1: COMPILING ALL Q2 C PROGRAMS WITH GCC"
echo "============================================================"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/paging_translation.c" -o "$BUILD_DIR/paging_translation_1024"
gcc -std=c11 -Wall -Wextra -DPAGE_SIZE=2048 "$SRC_DIR/paging_translation.c" -o "$BUILD_DIR/paging_translation_2048"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/paging_experiments.c" -o "$BUILD_DIR/paging_experiments"
gcc -std=c11 -Wall -Wextra "$SRC_DIR/system_memory_info.c" -o "$BUILD_DIR/system_memory_info"
echo "All programs compiled successfully into: $BUILD_DIR"

echo -e "\n============================================================"
echo " STEP 2: ADDRESS TRANSLATION (PAGE_SIZE = 1024)"
echo "============================================================"
"$BUILD_DIR/paging_translation_1024"

echo -e "\n============================================================"
echo " STEP 3: ADDRESS TRANSLATION (PAGE_SIZE = 2048)"
echo "============================================================"
"$BUILD_DIR/paging_translation_2048"

echo -e "\n============================================================"
echo " STEP 4: FRAGMENTATION, TLB EMAT & PAGE-TABLE SIZE EXPERIMENTS"
echo "============================================================"
"$BUILD_DIR/paging_experiments"

echo -e "\n============================================================"
echo " STEP 5: LINUX SYSTEM PAGE SIZE (getconf PAGESIZE)"
echo "============================================================"
echo -n "getconf PAGESIZE = "
getconf PAGESIZE

echo -e "\n============================================================"
echo " STEP 6: PAGE FAULTS & MEMORY USAGE (/usr/bin/time -v)"
echo "============================================================"
/usr/bin/time -v "$BUILD_DIR/system_memory_info"
