/** @file
  Shared UEFI memory type classification and display names.

  UEFI memory type display names are shared between the viewer and ranges.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __UEFI_MEMORY_TYPE_H__
#define __UEFI_MEMORY_TYPE_H__

#include "Base.h"

enum {
    UEFI_MEMORY_TYPE_RESERVED = 0,
    UEFI_MEMORY_TYPE_LOADER_CODE = 1,
    UEFI_MEMORY_TYPE_LOADER_DATA = 2,
    UEFI_MEMORY_TYPE_BOOT_SERVICES_CODE = 3,
    UEFI_MEMORY_TYPE_BOOT_SERVICES_DATA = 4,
    UEFI_MEMORY_TYPE_RUNTIME_SERVICES_CODE = 5,
    UEFI_MEMORY_TYPE_RUNTIME_SERVICES_DATA = 6,
    UEFI_MEMORY_TYPE_CONVENTIONAL = 7,
    UEFI_MEMORY_TYPE_UNUSABLE = 8,
    UEFI_MEMORY_TYPE_ACPI_RECLAIM = 9,
    UEFI_MEMORY_TYPE_ACPI_NVS = 10,
    UEFI_MEMORY_TYPE_MMIO = 11,
    UEFI_MEMORY_TYPE_MMIO_PORT_SPACE = 12,
    UEFI_MEMORY_TYPE_PAL_CODE = 13
};

/**
  Format the display name for a UEFI memory type.

  Unknown types retain their numeric value in the formatted name as
  Unknown(<type>).
**/
void
UefiMemoryTypeFormat(
    NX_U32   Type,
    NX_CHAR *Name,
    NX_UINTN NameBufferSize
    );

void
UefiMemoryAccessFormat(
    const NX_MEMORY_ACCESS *Access,
    NX_CHAR                *Buffer,
    NX_UINTN                BufferSize
    );

#endif
