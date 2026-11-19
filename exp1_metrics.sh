#!/usr/bin/env bash
set -euo pipefail

# Experiment 1: Compare executable sizes, sections, ldd dependencies,
# startup time, and memory usage.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"

cd "$BUILD_DIR"

echo "============================================================"
echo " EXPERIMENT 1: SIZE, DEPENDENCIES, STARTUP TIME & MEMORY"
echo "============================================================"

echo -e "\n--- 1. File Sizes (ls -lh) ---"
ls -lh bank_static bank_dynamic bank_dlopen libencryption.a libencryption.so

echo -e "\n--- 2. ELF Section Sizes (size) ---"
size bank_static bank_dynamic bank_dlopen

echo -e "\n--- 3. Shared Library Dependencies (ldd) ---"
echo "[bank_static]"
ldd bank_static || true
echo ""
echo "[bank_dynamic]"
ldd bank_dynamic
echo ""
echo "[bank_dlopen]"
ldd bank_dlopen

echo -e "\n--- 4. Startup Time & Peak Memory (/usr/bin/time -v) ---"
echo "[bank_static]"
/usr/bin/time -v ./bank_static > /dev/null 2>&1 || true
/usr/bin/time -v ./bank_static 2>&1 >/dev/null | grep -E "Elapsed|Maximum resident|Minor|Major"

echo ""
echo "[bank_dynamic]"
/usr/bin/time -v ./bank_dynamic 2>&1 >/dev/null | grep -E "Elapsed|Maximum resident|Minor|Major"

echo ""
echo "[bank_dlopen (with encryption)]"
/usr/bin/time -v ./bank_dlopen 2>&1 >/dev/null | grep -E "Elapsed|Maximum resident|Minor|Major"

echo ""
echo "[bank_dlopen (--no-encrypt)]"
/usr/bin/time -v ./bank_dlopen --no-encrypt 2>&1 >/dev/null | grep -E "Elapsed|Maximum resident|Minor|Major"
