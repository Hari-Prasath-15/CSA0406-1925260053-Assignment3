#!/usr/bin/env bash
set -euo pipefail

# Experiment 4: Test what happens when libencryption.so is missing.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"

cd "$BUILD_DIR"

echo "============================================================"
echo " EXPERIMENT 4: MISSING SHARED LIBRARY BEHAVIOUR"
echo "============================================================"

mv libencryption.so libencryption.so.missing_bak
echo "Temporarily renamed libencryption.so -> libencryption.so.missing_bak"

echo -e "\n--- 1. Running bank_static (Expected: Succeeds) ---"
./bank_static | grep -E "Version|Integrity"

echo -e "\n--- 2. Running bank_dynamic (Expected: Fails at load time) ---"
./bank_dynamic || echo "  -> bank_dynamic exited with status $?"

echo -e "\n--- 3. Running bank_dlopen --no-encrypt (Expected: Succeeds) ---"
./bank_dlopen --no-encrypt | grep -E "Operation|Library Status"

echo -e "\n--- 4. Running bank_dlopen (encryption requested, Expected: Graceful error) ---"
./bank_dlopen || echo "  -> bank_dlopen handled missing library cleanly and exited with status $?"

mv libencryption.so.missing_bak libencryption.so
echo -e "\nRestored libencryption.so."
