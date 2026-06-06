#!/bin/bash

set -e

# Core unit test driver.
# This script validates only Core layer logic on a Linux host,
# with no UEFI or UI runtime dependencies.

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

mkdir -p build

gcc -std=c11 -Wall -Wextra -Werror \
    -DCORE_UNIT_TEST \
    -I UefiNexusPkg/Tests/HostShim \
    -I UefiNexusPkg \
    UefiNexusPkg/Core/Mem/CursorEngine.c \
    UefiNexusPkg/Core/Mem/LayoutEngine.c \
    UefiNexusPkg/Core/Mem/FormatEngine.c \
    UefiNexusPkg/Core/Mem/AddressMap.c \
    UefiNexusPkg/Core/Mem/StateMachine.c \
    UefiNexusPkg/Tests/Core/test_cursor.c \
    UefiNexusPkg/Tests/Core/test_layout.c \
    UefiNexusPkg/Tests/Core/test_format.c \
    UefiNexusPkg/Tests/Core/test_state.c \
    UefiNexusPkg/Tests/Core/test_addressmap.c \
    UefiNexusPkg/Tests/Core/test_runner.c \
    -o build/core_unit_tests

./build/core_unit_tests
