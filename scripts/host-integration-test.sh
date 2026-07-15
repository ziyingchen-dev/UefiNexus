#!/bin/bash
# SPDX-License-Identifier: BSD-2-Clause-Patent

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

mkdir -p build

gcc -std=c11 -Wall -Wextra -Werror \
    -I UefiNexusPkg \
    UefiNexusPkg/UI/Pages/PageMem/Core/CursorEngine.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/LayoutEngine.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/FormatEngine.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/AddressMap.c \
    UefiNexusPkg/UI/Pages/PageMem/Core/StateMachine.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemView.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemController.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemActions.c \
    UefiNexusPkg/Tests/Mock/MemMock.c \
    UefiNexusPkg/Tests/Mock/TuiMock.c \
    UefiNexusPkg/Tests/Mock/ErrorMock.c \
    UefiNexusPkg/Tests/Core/test_integration.c \
    UefiNexusPkg/Tests/Core/integration_runner.c \
    -o build/host-integration-test

if [ -x build/host-integration-test ]; then
    ./build/host-integration-test
else
    echo "Error: build/host-integration-test not found or not executable"
    exit 1
fi