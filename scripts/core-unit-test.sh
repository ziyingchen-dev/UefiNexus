#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause-Patent

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

mkdir -p build

gcc -std=c11 -Wall -Wextra -Werror -DCORE_UNIT_TEST \
    -I UefiNexusPkg \
    UefiNexusPkg/UI/Pages/PageMem/Core/CursorEngine.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/LayoutEngine.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/FormatEngine.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/AddressMap.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/StateMachine.c \
    UefiNexusPkg/Tests/Core/test_cursor.c \
    UefiNexusPkg/Tests/Core/test_layout.c \
    UefiNexusPkg/Tests/Core/test_format.c \
    UefiNexusPkg/Tests/Core/test_state.c \
    UefiNexusPkg/Tests/Core/test_addressmap.c \
    UefiNexusPkg/Tests/Core/test_runner.c \
    -o build/core-unit-test

if [ -x build/core-unit-test ]; then
    ./build/core-unit-test
else
    echo "Error: build/core-unit-test not found or not executable"
    exit 1
fi