#!/usr/bin/env bash
set -euo pipefail

# Safely collects Linux disk, partition, and swap information (read-only).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"
mkdir -p results

DISK_OUT="results/disk_information.txt"
SWAP_OUT="results/swap_information.txt"

{
    echo "=========================================================================="
    echo " LINUX DISK AND PARTITION INFORMATION (Read-Only Inspection)"
    echo "=========================================================================="
    echo "Date / Host Kernel: $(uname -a)"
    echo ""

    echo "--- 1. lsblk (Block Devices & Partitions) ---"
    if command -v lsblk >/dev/null 2>&1; then
        lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT || lsblk || echo "lsblk returned non-zero status."
    else
        echo "lsblk command is not installed in this environment."
    fi

    echo ""
    echo "--- 2. fdisk -l (Partition Table Details) ---"
    if command -v fdisk >/dev/null 2>&1; then
        fdisk -l 2>&1 || echo "(Note: fdisk -l may require sudo on Ubuntu WSL to read raw block devices.)"
    else
        echo "fdisk command is not installed in this environment."
    fi

    echo ""
    echo "--- 3. df -h (Mounted Filesystems) ---"
    df -h || true
} > "$DISK_OUT"

{
    echo "=========================================================================="
    echo " LINUX SWAP SPACE INFORMATION (Read-Only Inspection)"
    echo "=========================================================================="
    echo ""

    echo "--- 1. swapon --show ---"
    if command -v swapon >/dev/null 2>&1; then
        SWAP_SHOW="$(swapon --show 2>&1 || true)"
        if [ -n "$SWAP_SHOW" ]; then
            echo "$SWAP_SHOW"
        else
            echo "(No active swap areas reported by swapon --show.)"
        fi
    else
        echo "swapon command is not installed in this environment."
    fi

    echo ""
    echo "--- 2. /proc/swaps ---"
    if [ -r /proc/swaps ]; then
        cat /proc/swaps
    else
        echo "/proc/swaps is not readable in this environment."
    fi

    echo ""
    echo "--- 3. free -h (RAM & Swap Summary) ---"
    if command -v free >/dev/null 2>&1; then
        free -h
    else
        echo "free command is not installed."
    fi
} > "$SWAP_OUT"

echo "Saved disk information to: $DISK_OUT"
echo "Saved swap information to: $SWAP_OUT"
