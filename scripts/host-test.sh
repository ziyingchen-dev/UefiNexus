#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause-Patent

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

mkdir -p build

gcc -std=c11 -Wall -Wextra -Werror \
    -I UefiNexusPkg \
    UefiNexusPkg/Core/Page/CursorEngine.c \
    UefiNexusPkg/Core/Page/LayoutEngine.c \
    UefiNexusPkg/Core/Page/FormatEngine.c \
    UefiNexusPkg/Core/Page/AddressMap.c \
    UefiNexusPkg/Core/Page/StateMachine.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemView.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemController.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemActions.c \
    UefiNexusPkg/Tests/Mock/MemMock.c \
    UefiNexusPkg/Tests/Mock/TuiMock.c \
    UefiNexusPkg/Tests/Mock/ErrorMock.c \
    UefiNexusPkg/Tests/Core/test_integration.c \
    UefiNexusPkg/Tests/Core/integration_runner.c \
    -o build/host_integration_tests


# Run the compiled integration test binary
if [ -x build/host_integration_tests ]; then
    ./build/host_integration_tests
else
    echo "Error: build/host_integration_tests not found or not executable"
    exit 1
fi
