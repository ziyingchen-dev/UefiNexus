#!/bin/bash

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$REPO_ROOT"

mkdir -p build

gcc -std=c11 -Wall -Wextra -Werror \
    -I UefiNexusPkg/Tests/HostShim \
    -I UefiNexusPkg \
    UefiNexusPkg/Core/Mem/CursorEngine.c \
    UefiNexusPkg/Core/Mem/LayoutEngine.c \
    UefiNexusPkg/Core/Mem/FormatEngine.c \
    UefiNexusPkg/Core/Mem/AddressMap.c \
    UefiNexusPkg/Core/Mem/StateMachine.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemView.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemController.c \
    UefiNexusPkg/UI/Pages/PageMem/PageMemActions.c \
    UefiNexusPkg/Tests/Mock/MemMock.c \
    UefiNexusPkg/Tests/Mock/TuiMock.c \
    UefiNexusPkg/Tests/Mock/UefiMock.c \
    UefiNexusPkg/Tests/Core/test_integration.c \
    UefiNexusPkg/Tests/Core/integration_runner.c \
    -o build/host_integration_tests


./build/unit_tests
