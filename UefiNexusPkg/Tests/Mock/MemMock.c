/**
 * MemMock.c - Mock implementation of Memory Adapter for testing
 */

#include "../../Adapter/AdapterInterface.h"

static PM_U64 gMockMemoryBuffer[256];  // Simulate 256 8-byte values
static PM_BOOL gMockMemoriesInitialized = PM_FALSE;
static PM_MEMORY_DESCRIPTOR gMockDescriptors[2] = {
    { PM_MEM_CONVENTIONAL, 0x1000, 1, 0 },
    { PM_MEM_OTHER, 0x2000, 1, 0 },
};

static PM_UI_STATUS MockMemInit(void);
static PM_BOOL MockMemIsAddressValid(PM_U64 Address);
static PM_U64 MockMemRead(PM_U64 Address, PM_UINTN Width);
static PM_UI_STATUS MockMemWrite(PM_U64 Address, PM_UINTN Width, PM_U64 Value);
static const PM_MEMORY_DESCRIPTOR* MockMemGetMemoryMapDescriptors(PM_UINTN *DescriptorCount);
static void MockMemDumpValidRanges(void);
static PM_UINTN gMockDumpCallCount = 0;

static MEMORY_ADAPTER_INTERFACE mMemoryAdapterMock = {
    .Init = MockMemInit,
    .MemRead = MockMemRead,
    .MemWrite = MockMemWrite,
    .GetMemoryMapDescriptors = MockMemGetMemoryMapDescriptors,
    .DumpValidRanges = MockMemDumpValidRanges,
};

static
PM_UI_STATUS
MockMemInit(void)
{
    // Initialize mock buffer with test pattern
    for (int i = 0; i < 256; i++) {
        gMockMemoryBuffer[i] = 0x0102030405060708 + i;
    }
    gMockMemoriesInitialized = PM_TRUE;
    return PM_UI_SUCCESS;
}

static
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

static
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

static
const PM_MEMORY_DESCRIPTOR*
MockMemGetMemoryMapDescriptors(PM_UINTN *DescriptorCount)
{
    if (DescriptorCount != NULL) {
        *DescriptorCount = 2;
    }

    return gMockDescriptors;
}

static
void
MockMemDumpValidRanges(void)
{
    gMockDumpCallCount++;
}

/**
 * Get mock memory adapter
 */
MEMORY_ADAPTER_INTERFACE*
GetMockMemoryAdapter(void)
{
    return &mMemoryAdapterMock;
}

static PM_BOOL
MockMemIsAddressValid(PM_U64 Address)
{
    return (Address >= 0x1000 && Address < 0x1000 + (256 * 8)) ? PM_TRUE : PM_FALSE;
}

/**
 * Set mock memory value for testing
 */
void
MockMemorySetValue(PM_U64 Address, PM_U64 Value)
{
    MockMemWrite(Address, 8, Value);
}

/**
 * Get mock memory value for verification
 */
PM_U64
MockMemoryGetValue(PM_U64 Address)
{
    return MockMemRead(Address, 8);
}

PM_UINTN
MockMemoryGetDumpCallCount(void)
{
    return gMockDumpCallCount;
}

void
MockMemoryReset(void)
{
    gMockMemoriesInitialized = PM_FALSE;
    gMockDumpCallCount = 0;
}
