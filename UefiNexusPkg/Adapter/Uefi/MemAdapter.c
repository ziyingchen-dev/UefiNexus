/**
 * MemAdapter.c - Wraps NexusMemLib to implement MEMORY_ADAPTER_INTERFACE
 * This adapter abstracts memory access for testability
 */

#include "../AdapterInterface.h"
#include "UefiBridge.h"

static PM_UI_STATUS MemAdapterInit(void);
static PM_U64 MemAdapterMemRead(PM_U64 Address, PM_UINTN Width);
static PM_UI_STATUS MemAdapterMemWrite(PM_U64 Address, PM_UINTN Width, PM_U64 Value);
static const PM_MEMORY_DESCRIPTOR* MemAdapterGetMemoryMapDescriptors(PM_UINTN *DescriptorCount);
static void MemAdapterDumpValidRanges(void);

static PM_MEMORY_DESCRIPTOR gCoreDescriptors[128];
static PM_UINTN gCoreDescriptorCount = 0;

static
PM_MEMORY_TYPE
MapMemoryType(PM_U32 Type)
{
    switch (Type) {
    case 1:
        return PM_MEM_LOADER_CODE;
    case 2:
        return PM_MEM_LOADER_DATA;
    case 3:
        return PM_MEM_BOOT_SERVICES_CODE;
    case 4:
        return PM_MEM_BOOT_SERVICES_DATA;
    case 5:
        return PM_MEM_RUNTIME_SERVICES_CODE;
    case 6:
        return PM_MEM_RUNTIME_SERVICES_DATA;
    case 7:
        return PM_MEM_CONVENTIONAL;
    case 11:
        return PM_MEM_MMIO;
    default:
        return PM_MEM_OTHER;
    }
}

static MEMORY_ADAPTER_INTERFACE mMemoryAdapter = {
    .Init = MemAdapterInit,
    .MemRead = MemAdapterMemRead,
    .MemWrite = MemAdapterMemWrite,
    .GetMemoryMapDescriptors = MemAdapterGetMemoryMapDescriptors,
    .DumpValidRanges = MemAdapterDumpValidRanges,
};

static
PM_UI_STATUS
MemAdapterInit(void)
{
    PM_UI_STATUS Status;
    PM_UINTN MemoryMapSize;
    PM_UINTN DescriptorSize;
    const PM_BRIDGE_MEMORY_DESCRIPTOR *Descriptors;

    Status = UefiBridgeInitMemoryMap();
    if (PM_UI_ERROR(Status)) {
        return Status;
    }

    Descriptors = UefiBridgeGetMemoryMapDescriptors(&MemoryMapSize, &DescriptorSize);
    if (Descriptors == NULL || DescriptorSize == 0) {
        gCoreDescriptorCount = 0;
        return PM_UI_NOT_FOUND;
    }

    gCoreDescriptorCount = MemoryMapSize / DescriptorSize;
    if (gCoreDescriptorCount > (sizeof(gCoreDescriptors) / sizeof(gCoreDescriptors[0]))) {
        gCoreDescriptorCount = sizeof(gCoreDescriptors) / sizeof(gCoreDescriptors[0]);
    }

    for (PM_UINTN Index = 0; Index < gCoreDescriptorCount; Index++) {
        const PM_BRIDGE_MEMORY_DESCRIPTOR *Source;

        Source = (const PM_BRIDGE_MEMORY_DESCRIPTOR *)((const PM_U8 *)Descriptors + (Index * DescriptorSize));
        gCoreDescriptors[Index].Type = MapMemoryType(Source->Type);
        gCoreDescriptors[Index].PhysicalStart = Source->PhysicalStart;
        gCoreDescriptors[Index].NumberOfPages = Source->NumberOfPages;
        gCoreDescriptors[Index].Attribute = Source->Attribute;
    }

    return PM_UI_SUCCESS;
}

static
PM_U64
MemAdapterMemRead(PM_U64 Address, PM_UINTN Width)
{
    return UefiBridgeMemRead(Address, Width);
}

static
PM_UI_STATUS
MemAdapterMemWrite(PM_U64 Address, PM_UINTN Width, PM_U64 Value)
{
    return UefiBridgeMemWrite(Address, Width, Value);
}

static
const PM_MEMORY_DESCRIPTOR*
MemAdapterGetMemoryMapDescriptors(PM_UINTN *DescriptorCount)
{
    if (DescriptorCount != NULL) {
        *DescriptorCount = gCoreDescriptorCount;
    }

    return gCoreDescriptors;
}

static
void
MemAdapterDumpValidRanges(void)
{
    UefiBridgeDumpMemoryRanges();
}

/**
 * Get the memory adapter interface instance
 */
MEMORY_ADAPTER_INTERFACE*
GetMemoryAdapter(void)
{
    return &mMemoryAdapter;
}
