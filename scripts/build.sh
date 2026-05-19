#!/usr/bin/env bash
set -euo pipefail

ARCH="${1:-}"

if [[ "$ARCH" != "x64" && "$ARCH" != "arm" ]]; then
    echo "Usage: ./scripts/build.sh <x64|arm>"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

EDK_DIR="${EDK2_PATH:-$REPO_ROOT/edk2}"

if [[ ! -d "$EDK_DIR" ]]; then
    echo "ERROR: edk2 not found"
    exit 1
fi

export WORKSPACE="$EDK_DIR"
export EDK_TOOLS_PATH="$EDK_DIR/BaseTools"
export PACKAGES_PATH="$REPO_ROOT"
export PATH="$EDK_DIR/BaseTools/BinWrappers/PosixLike:$PATH"

PKG=UefiNexusPkg
TARGET=DEBUG
TOOLCHAIN=GCC

if [[ "$ARCH" == "x64" ]]; then
    EDK_ARCH=X64
else
    EDK_ARCH=AARCH64
    export GCC_AARCH64_PREFIX=aarch64-linux-gnu-
fi

cd "$EDK_DIR"

build \
    -p "$PKG/$PKG.dsc" \
    -a "$EDK_ARCH" \
    -t "$TOOLCHAIN" \
    -b "$TARGET"