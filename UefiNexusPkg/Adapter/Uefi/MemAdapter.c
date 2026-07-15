/** @file
  Memory adapter for UefiNexus.

  This module implements MEMORY_ADAPTER_INTERFACE by forwarding memory map,
  memory read, and memory write operations to the UEFI bridge layer. It also
  converts UEFI memory descriptor types into framework-defined memory types so
  Core logic can remain independent from UEFI-specific definitions.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AdapterInterface.h"
#include "UefiBridge.h"

static NX_UI_STATUS MemAdapterInit(void);
static NX_U64 MemAdapterMemRead(NX_U64 Address, NX_UINTN Width);
static NX_UI_STATUS MemAdapterMemWrite(NX_U64 Address, NX_UINTN Width, NX_U64 Value);
static const NX_MEMORY_DESCRIPTOR *MemAdapterGetMemoryMapDescriptors(NX_UINTN *DescriptorCount);
static void MemAdapterDumpValidRanges(void);

//
// Cached Core-compatible memory descriptors.
//
static NX_MEMORY_DESCRIPTOR gCoreDescriptors[128];
static NX_UINTN             gCoreDescriptorCount = 0;

/**
  Convert a UEFI memory type value to a UefiNexus memory type.

  @param[in] Type  UEFI memory type value.

  @retval NX_MEM_LOADER_CODE            Loader code memory.
  @retval NX_MEM_LOADER_DATA            Loader data memory.
  @retval NX_MEM_BOOT_SERVICES_CODE     Boot services code memory.
  @retval NX_MEM_BOOT_SERVICES_DATA     Boot services data memory.
  @retval NX_MEM_RUNTIME_SERVICES_CODE  Runtime services code memory.
  @retval NX_MEM_RUNTIME_SERVICES_DATA  Runtime services data memory.
  @retval NX_MEM_CONVENTIONAL           Conventional memory.
  @retval NX_MEM_ACPI_RECLAIM           ACPI reclaim memory.
  @retval NX_MEM_ACPI_NVS               ACPI NVS memory.
  @retval NX_MEM_MMIO                   Memory-mapped I/O memory.
  @retval NX_MEM_OTHER                  Unsupported or unmapped memory type.
**/
static
NX_MEMORY_TYPE
MapMemoryType(
    NX_U32 Type
    )
{
    switch (Type) {
    case 1:
        return NX_MEM_LOADER_CODE;
    case 2:
        return NX_MEM_LOADER_DATA;
    case 3:
        return NX_MEM_BOOT_SERVICES_CODE;
    case 4:
        return NX_MEM_BOOT_SERVICES_DATA;
    case 5:
        return NX_MEM_RUNTIME_SERVICES_CODE;
    case 6:
        return NX_MEM_RUNTIME_SERVICES_DATA;
    case 7:
        return NX_MEM_CONVENTIONAL;
    case 9:
        return NX_MEM_ACPI_RECLAIM;
    case 10:
        return NX_MEM_ACPI_NVS;
    case 11:
    case 12:
        return NX_MEM_MMIO;
    default:
        return NX_MEM_OTHER;
    }
}

//
// Memory adapter interface instance.
//
static MEMORY_ADAPTER_INTERFACE mMemoryAdapter = {
    .Init                    = MemAdapterInit,
    .MemRead                 = MemAdapterMemRead,
    .MemWrite                = MemAdapterMemWrite,
    .GetMemoryMapDescriptors = MemAdapterGetMemoryMapDescriptors,
    .DumpValidRanges         = MemAdapterDumpValidRanges,
};

/**
  Initialize the memory adapter.

  This function initializes the bridge memory map, retrieves UEFI memory
  descriptors from the bridge layer, converts them into Core-compatible memory
  descriptors, and caches them for later use.

  @retval NX_UI_SUCCESS    The memory adapter was initialized successfully.
  @retval NX_UI_NOT_FOUND  Memory map descriptors are unavailable.
  @retval Others           Error returned by the UEFI bridge layer.
**/
static
NX_UI_STATUS
MemAdapterInit(void)
{
    NX_UI_STATUS                      Status;
    NX_UINTN                          MemoryMapSize;
    NX_UINTN                          DescriptorSize;
    const NX_BRIDGE_MEMORY_DESCRIPTOR *Descriptors;

    Status = UefiBridgeInitMemoryMap();
    if (NX_UI_ERROR(Status)) {
        return Status;
    }

    Descriptors = UefiBridgeGetMemoryMapDescriptors(&MemoryMapSize, &DescriptorSize);
    if (Descriptors == NULL || DescriptorSize == 0) {
        gCoreDescriptorCount = 0;
        return NX_UI_NOT_FOUND;
    }

    gCoreDescriptorCount = MemoryMapSize / DescriptorSize;
    if (gCoreDescriptorCount > (sizeof(gCoreDescriptors) / sizeof(gCoreDescriptors[0]))) {
        gCoreDescriptorCount = sizeof(gCoreDescriptors) / sizeof(gCoreDescriptors[0]);
    }

    for (NX_UINTN Index = 0; Index < gCoreDescriptorCount; Index++) {
        const NX_BRIDGE_MEMORY_DESCRIPTOR *Source;

        Source = (const NX_BRIDGE_MEMORY_DESCRIPTOR *)((const NX_U8 *)Descriptors + (Index * DescriptorSize));

        gCoreDescriptors[Index].Type          = MapMemoryType(Source->Type);
        gCoreDescriptors[Index].PhysicalStart = Source->PhysicalStart;
        gCoreDescriptors[Index].NumberOfPages = Source->NumberOfPages;
        gCoreDescriptors[Index].Attribute     = Source->Attribute;
    }

    return NX_UI_SUCCESS;
}

/**
  Read a value from a physical address.

  @param[in] Address  Physical address to read.
  @param[in] Width    Access width in bytes.

  @return Value read from the requested address.
**/
static
NX_U64
MemAdapterMemRead(
    NX_U64   Address,
    NX_UINTN Width
    )
{
    return UefiBridgeMemRead(Address, Width);
}

/**
  Write a value to a physical address.

  @param[in] Address  Physical address to write.
  @param[in] Width    Access width in bytes.
  @param[in] Value    Value to write.

  @retval NX_UI_SUCCESS  The write completed successfully.
  @retval Others         Error returned by the UEFI bridge layer.
**/
static
NX_UI_STATUS
MemAdapterMemWrite(
    NX_U64   Address,
    NX_UINTN Width,
    NX_U64   Value
    )
{
    return UefiBridgeMemWrite(Address, Width, Value);
}

/**
  Retrieve cached Core-compatible memory map descriptors.

  @param[out] DescriptorCount  Optional pointer that receives descriptor count.

  @retval NULL   No descriptor buffer is available.
  @retval Other  Pointer to cached NX_MEMORY_DESCRIPTOR array.
**/
static
const NX_MEMORY_DESCRIPTOR *
MemAdapterGetMemoryMapDescriptors(
    NX_UINTN *DescriptorCount
    )
{
    if (DescriptorCount != NULL) {
        *DescriptorCount = gCoreDescriptorCount;
    }

    return gCoreDescriptors;
}

/**
  Dump valid memory ranges through the bridge layer.

  This function is mainly used by UI/debug flows to display the memory ranges
  known to the current memory adapter implementation.
**/
static
void
MemAdapterDumpValidRanges(void)
{
    UefiBridgeDumpMemoryRanges();
}

/**
  Retrieve the memory adapter interface instance.

  @retval Other  Pointer to the static MEMORY_ADAPTER_INTERFACE instance.
**/
MEMORY_ADAPTER_INTERFACE *
GetMemoryAdapter(void)
{
    return &mMemoryAdapter;
}
