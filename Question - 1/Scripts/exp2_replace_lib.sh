#!/usr/bin/env bash
set -euo pipefail

# Experiment 2: Replace libencryption.so with Version 2.0 WITHOUT recompiling
# bank_dynamic or bank_static, and observe the difference.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"

cd "$BUILD_DIR"

echo "============================================================"
echo " EXPERIMENT 2: REPLACING SHARED LIBRARY WITHOUT RECOMPILING"
echo "============================================================"

echo -e "\n--- Step 1: Run bank_static and bank_dynamic with v1.0 ---"
./bank_static | grep -E "Version|Algorithm|Encrypted"
echo ""
./bank_dynamic | grep -E "Version|Algorithm|Encrypted"

echo -e "\n--- Step 2: Compile v2.0 (encryption_v2.c) directly over libencryption.so ---"
cp libencryption.so libencryption.so.v1.bak
gcc -std=c11 -Wall -Wextra -fPIC -shared "$ROOT_DIR/library/encryption_v2.c" -o "$BUILD_DIR/libencryption.so"
echo "Replaced libencryption.so with v2.0. Main executables were NOT recompiled."

echo -e "\n--- Step 3: Re-run bank_static and bank_dynamic ---"
echo "[bank_static] (Still uses v1.0 because code was copied at compile time):"
./bank_static | grep -E "Version|Algorithm|Encrypted"
echo ""
echo "[bank_dynamic] (Automatically uses v2.0 without recompiling!):"
./bank_dynamic | grep -E "Version|Algorithm|Encrypted"
echo ""
echo "[bank_dlopen] (Also automatically uses v2.0 at runtime!):"
./bank_dlopen | grep -E "Version|Algorithm|Encrypted"

echo -e "\n--- Step 4: Restoring v1.0 libencryption.so ---"
mv libencryption.so.v1.bak libencryption.so
echo "Restored original libencryption.so (v1.0)."
