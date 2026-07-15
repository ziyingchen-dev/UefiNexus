/** @file
  Host-side UEFI and TUI stubs.

  This module provides minimal host-side implementations for selected UEFI Boot
  Services and TuiLib functions. These stubs allow adapter-boundary or
  integration tests to link without a real UEFI firmware environment.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <stdio.h>
#include <stdarg.h>

#include "Library/UefiBootServicesTableLib.h"
#include "Library/TuiLib.h"

/**
  Append captured text output to the host-side mock TUI buffer.

  Implemented by the mock TUI layer.
**/
extern VOID
MockTuiAppendString(
    IN CONST CHAR16 *String
    );

/**
  Host stub for EFI_BOOT_SERVICES.FreePool().

  The host shim does not own real UEFI pool allocations, so this stub accepts
  the call and returns success.
**/
STATIC
EFI_STATUS
EFIAPI
HostFreePool(
    IN VOID *Buffer
    )
{
    (void)Buffer;

    return EFI_SUCCESS;
}

/**
  Host stub for EFI_BOOT_SERVICES.GetMemoryMap().

  This stub only models the first-call behavior used by InitMemoryMap(), where
  the caller expects EFI_BUFFER_TOO_SMALL before allocating a buffer.
**/
STATIC
EFI_STATUS
EFIAPI
HostGetMemoryMap(
    OUT UINTN                 *MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN                 *MapKey,
    OUT UINTN                 *DescriptorSize,
    OUT UINT32                *DescriptorVersion
    )
{
    (void)MemoryMap;
    (void)MapKey;
    (void)DescriptorSize;
    (void)DescriptorVersion;

    if (MemoryMapSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    return EFI_BUFFER_TOO_SMALL;
}

/**
  Host stub for EFI_BOOT_SERVICES.AllocatePool().

  This stub does not allocate memory. It clears the output pointer and returns
  success to satisfy host-side link and flow requirements.
**/
STATIC
EFI_STATUS
EFIAPI
HostAllocatePool(
    IN UINT32 PoolType,
    IN UINTN  Size,
    OUT VOID  **Buffer
    )
{
    (void)PoolType;
    (void)Size;

    if (Buffer != NULL) {
        *Buffer = NULL;
    }

    return EFI_SUCCESS;
}

EFI_BOOT_SERVICES mHostBootServices = {
    .FreePool     = HostFreePool,
    .GetMemoryMap = HostGetMemoryMap,
    .AllocatePool = HostAllocatePool,
};

EFI_BOOT_SERVICES *gBS = &mHostBootServices;

VOID
TuiClearScreen(
    VOID
    )
{
    return;
}

VOID
TuiWaitForKeyPress(
    VOID
    )
{
    return;
}

VOID
TuiDrawHeader(
    CONST CHAR16 *Title
    )
{
    (void)Title;
}

VOID
TuiDrawFooter(
    CONST CHAR16 *Help
    )
{
    (void)Help;
}

VOID
TuiSetAttribute(
    UINTN Attribute
    )
{
    (void)Attribute;
}

VOID
TuiSetCursorPosition(
    UINTN Column,
    UINTN Row
    )
{
    (void)Column;
    (void)Row;
}

/**
  Host-side Print() implementation.

  This captures formatted wide-character output and forwards it into the mock
  TUI capture buffer for tests.
**/
UINTN
Print(
    IN CONST CHAR16 *Format,
    ...
    )
{
    CHAR16  Buffer[1024];
    int     Result;
    va_list Args;

    if (Format == NULL) {
        return 0;
    }

    va_start(Args, Format);
    Result = vswprintf(Buffer, sizeof(Buffer) / sizeof(CHAR16), Format, Args);
    va_end(Args);

    if (Result > 0) {
        MockTuiAppendString(Buffer);
    }

    return (Result < 0) ? 0 : (UINTN)Result;
}