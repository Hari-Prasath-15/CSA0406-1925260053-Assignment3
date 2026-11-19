#!/usr/bin/env bash
set -euo pipefail

# Build script for Q1: Static Linking, Dynamic Linking, and Runtime dlopen()
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$ROOT_DIR/source"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

echo "=== [1/5] Building Static Library (libencryption.a) ==="
gcc -std=c11 -Wall -Wextra -c "$SRC_DIR/encryption.c" -o "$BUILD_DIR/encryption_static.o"
ar rcs "$BUILD_DIR/libencryption.a" "$BUILD_DIR/encryption_static.o"

echo "=== [2/5] Building Shared Library v1.0 (libencryption.so) with -fPIC -shared ==="
gcc -std=c11 -Wall -Wextra -fPIC -shared "$SRC_DIR/encryption.c" -o "$BUILD_DIR/libencryption.so"

echo "=== [3/5] Building Statically Linked Executable (bank_static) ==="
# Note: -static links both libencryption.a and static libc.
# If your system lacks static libc (libc.a), remove -static to link only libencryption.a statically.
if gcc -std=c11 -Wall -Wextra -static "$SRC_DIR/main.c" "$SRC_DIR/maps_helper.c" \
       "$BUILD_DIR/libencryption.a" -o "$BUILD_DIR/bank_static" 2>/dev/null; then
    echo "  -> Built fully static executable: $BUILD_DIR/bank_static"
else
    echo "  -> Static libc not found; linking libencryption.a statically with dynamic libc."
    gcc -std=c11 -Wall -Wextra "$SRC_DIR/main.c" "$SRC_DIR/maps_helper.c" \
        "$BUILD_DIR/libencryption.a" -o "$BUILD_DIR/bank_static"
fi

echo "=== [4/5] Building Dynamically Linked Executable (bank_dynamic) ==="
gcc -std=c11 -Wall -Wextra "$SRC_DIR/main.c" "$SRC_DIR/maps_helper.c" \
    -L"$BUILD_DIR" -lencryption -Wl,-rpath,'$ORIGIN' -o "$BUILD_DIR/bank_dynamic"

echo "=== [5/5] Building Runtime-Loaded Executable (bank_dlopen) ==="
gcc -std=c11 -Wall -Wextra "$SRC_DIR/main_dlopen.c" "$SRC_DIR/maps_helper.c" \
    -ldl -o "$BUILD_DIR/bank_dlopen"

echo ""
echo "Build completed successfully! Binaries are in: $BUILD_DIR"
ls -lh "$BUILD_DIR/bank_static" "$BUILD_DIR/bank_dynamic" "$BUILD_DIR/bank_dlopen" \
       "$BUILD_DIR/libencryption.a" "$BUILD_DIR/libencryption.so"
