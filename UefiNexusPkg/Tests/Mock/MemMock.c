/**
 * MemMock.c - Mock implementation of Memory Adapter for testing
 */

#include <Uefi.h>
#include "../../Adapter/AdapterInterface.h"

STATIC UINT64 gMockMemoryBuffer[256];  // Simulate 256 8-byte values
STATIC BOOLEAN gMockMemoriesInitialized = FALSE;
STATIC PM_MEMORY_DESCRIPTOR gMockDescriptors[2] = {
    { PM_MEM_CONVENTIONAL, 0x1000, 1, 0 },
    { PM_MEM_OTHER, 0x2000, 1, 0 },
};

STATIC PM_UI_STATUS MockMemInit(VOID);
BOOLEAN MockMemIsAddressValid(UINT64 Address);
STATIC PM_U64 MockMemRead(PM_U64 Address, PM_UINTN Width);
STATIC PM_UI_STATUS MockMemWrite(PM_U64 Address, PM_UINTN Width, PM_U64 Value);
STATIC CONST PM_MEMORY_DESCRIPTOR* MockMemGetMemoryMapDescriptors(PM_UINTN *DescriptorCount);
STATIC VOID MockMemDumpValidRanges(VOID);
STATIC UINTN gMockDumpCallCount = 0;

STATIC MEMORY_ADAPTER_INTERFACE mMemoryAdapterMock = {
    .Init = MockMemInit,
    .MemRead = MockMemRead,
    .MemWrite = MockMemWrite,
    .GetMemoryMapDescriptors = MockMemGetMemoryMapDescriptors,
    .DumpValidRanges = MockMemDumpValidRanges,
};

STATIC
PM_UI_STATUS
MockMemInit(VOID)
{
    // Initialize mock buffer with test pattern
    for (int i = 0; i < 256; i++) {
        gMockMemoryBuffer[i] = 0x0102030405060708 + i;
    }
    gMockMemoriesInitialized = TRUE;
    return PM_UI_SUCCESS;
}

STATIC
PM_U64
MockMemRead(PM_U64 Address, PM_UINTN Width)
{
    (void)Width;
    if (!MockMemIsAddressValid(Address)) {
        return 0;
    }

    // Map address to buffer index
    PM_UINTN Index = (Address - 0x1000) / 8;
    if (Index >= 256) return 0;

    return gMockMemoryBuffer[Index];
}

STATIC
PM_UI_STATUS
MockMemWrite(PM_U64 Address, PM_UINTN Width, PM_U64 Value)
{
    (void)Width;
    if (!MockMemIsAddressValid(Address)) {
        return PM_UI_ACCESS_DENIED;
    }

    // Map address to buffer index
    PM_UINTN Index = (Address - 0x1000) / 8;
    if (Index >= 256) {
        return PM_UI_ACCESS_DENIED;
    }

    gMockMemoryBuffer[Index] = Value;
    return PM_UI_SUCCESS;
}

STATIC
CONST PM_MEMORY_DESCRIPTOR*
MockMemGetMemoryMapDescriptors(PM_UINTN *DescriptorCount)
{
    if (DescriptorCount != NULL) {
        *DescriptorCount = 2;
    }

    return gMockDescriptors;
}

STATIC
VOID
MockMemDumpValidRanges(VOID)
{
    gMockDumpCallCount++;
}

/**
 * Get mock memory adapter
 */
MEMORY_ADAPTER_INTERFACE*
GetMockMemoryAdapter(VOID)
{
    return &mMemoryAdapterMock;
}

BOOLEAN
MockMemIsAddressValid(UINT64 Address)
{
    return Address >= 0x1000 && Address < 0x1000 + (256 * 8);
}

/**
 * Set mock memory value for testing
 */
VOID
MockMemorySetValue(UINT64 Address, UINT64 Value)
{
    MockMemWrite(Address, 8, Value);
}

/**
 * Get mock memory value for verification
 */
UINT64
MockMemoryGetValue(UINT64 Address)
{
    return MockMemRead(Address, 8);
}

UINTN
MockMemoryGetDumpCallCount(VOID)
{
    return gMockDumpCallCount;
}

VOID
MockMemoryReset(VOID)
{
    gMockMemoriesInitialized = FALSE;
    gMockDumpCallCount = 0;
}
