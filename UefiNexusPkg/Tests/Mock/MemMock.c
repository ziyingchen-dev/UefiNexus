/** @file
  Mock memory adapter for host-side tests.

  This module implements MEMORY_ADAPTER_INTERFACE for PageMem host-side
  integration tests. It provides a small simulated memory buffer, mock memory
  descriptors, read/write helpers, and counters used to verify UI actions.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../../Adapter/AdapterInterface.h"

//
// Simulated memory storage.
//
// The mock maps addresses starting at 0x1000 into this buffer. Each entry is
// one 64-bit value.
//
static NX_U64 gMockMemoryBuffer[256];

//
// Mock initialization state.
//
static NX_BOOL gMockMemoriesInitialized = NX_FALSE;

//
// Mock memory map descriptors.
//
// The first descriptor represents a valid conventional memory range.
// The second descriptor represents an invalid/other range for negative tests.
//
static NX_MEMORY_DESCRIPTOR gMockDescriptors[2] = {
    { NX_MEM_CONVENTIONAL, 0x1000, 1, 0 },
    { NX_MEM_OTHER,        0x2000, 1, 0 },
};

static NX_UI_STATUS MockMemInit(void);
static NX_BOOL MockMemIsAddressValid(NX_U64 Address);
static NX_U64 MockMemRead(NX_U64 Address, NX_UINTN Width);
static NX_UI_STATUS MockMemWrite(NX_U64 Address, NX_UINTN Width, NX_U64 Value);
static const NX_MEMORY_DESCRIPTOR *MockMemGetMemoryMapDescriptors(NX_UINTN *DescriptorCount);
static void MockMemDumpValidRanges(void);

//
// Number of times DumpValidRanges() was requested.
//
static NX_UINTN gMockDumpCallCount = 0;

//
// Mock memory adapter instance used by host-side integration tests.
//
static MEMORY_ADAPTER_INTERFACE mMemoryAdapterMock = {
    .Init                    = MockMemInit,
    .MemRead                 = MockMemRead,
    .MemWrite                = MockMemWrite,
    .GetMemoryMapDescriptors = MockMemGetMemoryMapDescriptors,
    .DumpValidRanges         = MockMemDumpValidRanges,
};

static
NX_UI_STATUS
MockMemInit(void)
{
    //
    // Initialize mock memory with deterministic test pattern.
    //
    for (int Index = 0; Index < 256; Index++) {
        gMockMemoryBuffer[Index] = 0x0102030405060708 + Index;
    }

    gMockMemoriesInitialized = NX_TRUE;

    return NX_UI_SUCCESS;
}

static
NX_U64
MockMemRead(
    NX_U64   Address,
    NX_UINTN Width
    )
{
    NX_UINTN Index;

    //
    // Width is ignored by this mock because storage is modeled as 64-bit slots.
    //
    (void)Width;

    if (!MockMemIsAddressValid(Address)) {
        return 0;
    }

    //
    // Map physical address to mock buffer index.
    //
    Index = (Address - 0x1000) / 8;
    if (Index >= 256) {
        return 0;
    }

    return gMockMemoryBuffer[Index];
}

static
NX_UI_STATUS
MockMemWrite(
    NX_U64   Address,
    NX_UINTN Width,
    NX_U64   Value
    )
{
    NX_UINTN Index;

    //
    // Width is ignored by this mock because storage is modeled as 64-bit slots.
    //
    (void)Width;

    if (!MockMemIsAddressValid(Address)) {
        return NX_UI_ACCESS_DENIED;
    }

    //
    // Map physical address to mock buffer index.
    //
    Index = (Address - 0x1000) / 8;
    if (Index >= 256) {
        return NX_UI_ACCESS_DENIED;
    }

    gMockMemoryBuffer[Index] = Value;

    return NX_UI_SUCCESS;
}

static
const NX_MEMORY_DESCRIPTOR *
MockMemGetMemoryMapDescriptors(
    NX_UINTN *DescriptorCount
    )
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
  Retrieve the mock memory adapter.

  @retval Other  Pointer to the mock MEMORY_ADAPTER_INTERFACE instance.
**/
MEMORY_ADAPTER_INTERFACE *
GetMockMemoryAdapter(void)
{
    return &mMemoryAdapterMock;
}

/**
  Check whether an address is inside the mock valid memory range.
**/
static
NX_BOOL
MockMemIsAddressValid(
    NX_U64 Address
    )
{
    return (Address >= 0x1000 && Address < 0x1000 + (256 * 8)) ?
           NX_TRUE :
           NX_FALSE;
}

/**
  Set a mock memory value for tests.

  @param[in] Address  Mock physical address.
  @param[in] Value    Value to write.
**/
void
MockMemorySetValue(
    NX_U64 Address,
    NX_U64 Value
    )
{
    MockMemWrite(Address, 8, Value);
}

/**
  Get a mock memory value for verification.

  @param[in] Address  Mock physical address.

  @return Value read from mock memory.
**/
NX_U64
MockMemoryGetValue(
    NX_U64 Address
    )
{
    return MockMemRead(Address, 8);
}

/**
  Get the DumpValidRanges() call count.

  @return Number of dump requests.
**/
NX_UINTN
MockMemoryGetDumpCallCount(void)
{
    return gMockDumpCallCount;
}

/**
  Reset mock memory adapter state.
**/
void
MockMemoryReset(void)
{
    gMockMemoriesInitialized = NX_FALSE;
    gMockDumpCallCount       = 0;
}