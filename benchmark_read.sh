#!/usr/bin/env bash
set -euo pipefail

# Safe, non-destructive disk read-speed benchmark using an ordinary temporary file.
# NEVER writes to raw disks or partitions.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"
mkdir -p results

OUT_FILE="results/disk_read_speed.txt"
TEST_FILE="/tmp/os_q6_read_benchmark_128mb.bin"
SIZE_MB=128

cleanup() {
    rm -f "$TEST_FILE"
}
trap cleanup EXIT

{
    echo "=========================================================================="
    echo " SAFE DISK READ-SPEED MEASUREMENT (Non-Destructive File Test)"
    echo "=========================================================================="
    echo "Test File Path : $TEST_FILE"
    echo "Test File Size : ${SIZE_MB} MB (ordinary file; no raw block device modified)"
    echo ""

    echo "--- Step 1: Creating ${SIZE_MB} MB ordinary test file ---"
    dd if=/dev/zero of="$TEST_FILE" bs=1M count="$SIZE_MB" conv=fdatasync 2>&1

    echo ""
    echo "--- Step 2: Measuring Read Speed (Attempting O_DIRECT, fallback to cached read) ---"
    if dd if="$TEST_FILE" of=/dev/null bs=1M iflag=direct 2>&1; then
        echo "Measurement Mode: Direct I/O (iflag=direct) - bypasses OS page cache."
    else
        echo "Note: iflag=direct not supported on this filesystem; using standard read."
        dd if="$TEST_FILE" of=/dev/null bs=1M 2>&1
        echo "Measurement Mode: Buffered/Cached read (includes OS page-cache effects)."
    fi

    echo ""
    echo "--- Storage Type Comparison Note ---"
    echo "Only one primary storage device (WSL virtual disk / container storage) was"
    echo "available during this run. Therefore, a direct SSD vs. HDD comparison on"
    echo "two separate physical drive types could not be performed automatically."
    echo "If an external HDD is mounted (e.g., /mnt/d), you can re-run dd on an"
    echo "ordinary file there to compare sequential read throughput."
} > "$OUT_FILE"

echo "Saved read-speed benchmark to: $OUT_FILE"
