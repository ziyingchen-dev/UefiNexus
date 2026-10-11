/** @file
  Shared UEFI memory type classification and display names.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "UefiMemoryType.h"

static const NX_CHAR *gUefiMemoryTypeNames[] = {
    [UEFI_MEMORY_TYPE_RESERVED]              = "EfiReservedMemoryType",
    [UEFI_MEMORY_TYPE_LOADER_CODE]           = "EfiLoaderCode",
    [UEFI_MEMORY_TYPE_LOADER_DATA]           = "EfiLoaderData",
    [UEFI_MEMORY_TYPE_BOOT_SERVICES_CODE]    = "EfiBootServicesCode",
    [UEFI_MEMORY_TYPE_BOOT_SERVICES_DATA]    = "EfiBootServicesData",
    [UEFI_MEMORY_TYPE_RUNTIME_SERVICES_CODE] = "EfiRuntimeServicesCode",
    [UEFI_MEMORY_TYPE_RUNTIME_SERVICES_DATA] = "EfiRuntimeServicesData",
    [UEFI_MEMORY_TYPE_CONVENTIONAL]          = "EfiConventionalMemory",
    [UEFI_MEMORY_TYPE_UNUSABLE]              = "EfiUnusableMemory",
    [UEFI_MEMORY_TYPE_ACPI_RECLAIM]          = "EfiACPIReclaimMemory",
    [UEFI_MEMORY_TYPE_ACPI_NVS]              = "EfiACPIMemoryNVS",
    [UEFI_MEMORY_TYPE_MMIO]                  = "EfiMemoryMappedIO",
    [UEFI_MEMORY_TYPE_MMIO_PORT_SPACE]       = "EfiMemoryMappedIOPortSpace",
    [UEFI_MEMORY_TYPE_PAL_CODE]              = "EfiPalCode"
};

static
void
CopyString(
    NX_CHAR       *Destination,
    NX_UINTN       DestinationSize,
    const NX_CHAR *Source
    )
{
    NX_UINTN Index;

    if (Destination == NULL || DestinationSize == 0) {
        return;
    }

    for (Index = 0; Index + 1 < DestinationSize && Source[Index] != '\0'; Index++) {
        Destination[Index] = Source[Index];
    }

    Destination[Index] = '\0';
}

static
void
FormatUnknownType(
    NX_U32   Type,
    NX_CHAR *Buffer,
    NX_UINTN BufferSize
    )
{
    NX_CHAR  Digits[10];
    NX_UINTN Count;
    NX_UINTN Position;

    if (Buffer == NULL || BufferSize == 0) {
        return;
    }

    CopyString(Buffer, BufferSize, "Unknown(");
    Position = 8;
    if (Position >= BufferSize) {
        Buffer[BufferSize - 1] = '\0';
        return;
    }

    Count = 0;
    do {
        Digits[Count++] = (NX_CHAR)('0' + (Type % 10));
        Type /= 10;
    } while (Type != 0 && Count < sizeof(Digits));

    while (Count > 0 && Position + 1 < BufferSize) {
        Buffer[Position++] = Digits[--Count];
    }

    if (Position + 1 < BufferSize) {
        Buffer[Position++] = ')';
    }

    Buffer[Position] = '\0';
}

void
UefiMemoryTypeFormat(
    NX_U32   Type,
    NX_CHAR *Name,
    NX_UINTN NameBufferSize
    )
{
    if (Type < (sizeof(gUefiMemoryTypeNames) / sizeof(gUefiMemoryTypeNames[0]))) {
        CopyString(Name, NameBufferSize, gUefiMemoryTypeNames[Type]);
        return;
    }

    FormatUnknownType(Type, Name, NameBufferSize);
}

void
UefiMemoryAccessFormat(
    const NX_MEMORY_ACCESS *Access,
    NX_CHAR                *Buffer,
    NX_UINTN                BufferSize
    )
{
    if (Buffer == NULL || BufferSize == 0) {
        return;
    }

    if (Access == NULL || !Access->AccessKnown) {
        CopyString(Buffer, BufferSize, "Unknown");
        return;
    }

    if (Access->ReadAllowed) {
        CopyString(
            Buffer,
            BufferSize,
            Access->WriteAllowed ? "R/W" : "R/-"
            );
    } else if (Access->WriteAllowed) {
        CopyString(Buffer, BufferSize, "-/W");
    } else {
        CopyString(Buffer, BufferSize, "--");
    }
}
