#!/usr/bin/env bash
set -euo pipefail

# Experiment 3: Run two copies of bank_dynamic simultaneously and inspect
# shared vs. private pages using pmap and /proc/<PID>/smaps.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"

cd "$BUILD_DIR"

echo "============================================================"
echo " EXPERIMENT 3: TWO COPIES & SHARED MEMORY PAGES"
echo "============================================================"

./bank_dynamic --hold 15 > /dev/null &
PID1=$!
./bank_dynamic --hold 15 > /dev/null &
PID2=$!

sleep 1
echo "Started two instances of bank_dynamic: PID1=$PID1, PID2=$PID2"

echo -e "\n--- pmap -x for PID1 ($PID1) [Filtered] ---"
pmap -x "$PID1" | grep -E "Address|bank_dynamic|libencryption|libc|total" || true

echo -e "\n--- /proc/$PID1/smaps for libencryption.so ---"
awk '/libencryption\.so/,/^VmFlags:/ { if ($0 ~ /libencryption|Size:|Rss:|Pss:|Shared_Clean:|Private_Dirty:/) print }' "/proc/$PID1/smaps"

echo -e "\n--- /proc/$PID2/smaps for libencryption.so ---"
awk '/libencryption\.so/,/^VmFlags:/ { if ($0 ~ /libencryption|Size:|Rss:|Pss:|Shared_Clean:|Private_Dirty:/) print }' "/proc/$PID2/smaps"

kill "$PID1" "$PID2" 2>/dev/null || true
wait "$PID1" "$PID2" 2>/dev/null || true
echo -e "\nTerminated both test instances cleanly."
