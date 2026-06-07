#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause-Patent

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

mkdir -p build

gcc -std=c11 -Wall -Wextra -Werror -DCORE_UNIT_TEST \
    -I UefiNexusPkg \
    UefiNexusPkg/Core/Page/CursorEngine.c \
    UefiNexusPkg/Core/Page/LayoutEngine.c \
    UefiNexusPkg/Core/Page/FormatEngine.c \
    UefiNexusPkg/Core/Page/AddressMap.c \
    UefiNexusPkg/Core/Page/StateMachine.c \
    UefiNexusPkg/Tests/Core/test_cursor.c \
    UefiNexusPkg/Tests/Core/test_layout.c \
    UefiNexusPkg/Tests/Core/test_format.c \
    UefiNexusPkg/Tests/Core/test_state.c \
    UefiNexusPkg/Tests/Core/test_addressmap.c \
    UefiNexusPkg/Tests/Core/test_runner.c \
    -o build/unit_tests

if [ -x build/unit_tests ]; then
    ./build/unit_tests
else
    echo "Error: build/unit_tests not found or not executable"
    exit 1
fi
