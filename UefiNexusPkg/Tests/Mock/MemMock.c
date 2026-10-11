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
static NX_U64  gMockReadFailureAddress = 0;
static NX_BOOL gMockReadFailureEnabled = NX_FALSE;
static NX_MEMORY_ACCESS gMockAccess[3];
static NX_UINTN gMockAccessQueryCount = 0;
static NX_UINTN gMockPageAccessQueryCount = 0;
static NX_U64 gMockLastAccessBase = 0;
static NX_U64 gMockLastAccessLength = 0;
static NX_U64 gMockLastPageAccessBase = 0;
static NX_U64 gMockLastPageAccessLength = 0;

//
// Mock memory map descriptors.
//
// The first descriptor represents a valid conventional memory range.
// The second descriptor represents an invalid/other range for negative tests.
//
static NX_MEMORY_DESCRIPTOR gMockDescriptors[3] = {
    { NX_MEM_CONVENTIONAL,  0x1000, 1, 0, 7 },
    { NX_MEM_OTHER,         0x2000, 1, 0, 14 },
    { NX_MEM_ACPI_RECLAIM,  0x3000, 1, 0, 9 },
};

static NX_UI_STATUS MockMemInit(void);
static NX_BOOL MockMemIsAddressValid(NX_U64 Address);
static NX_UI_STATUS MockMemRead(NX_U64 Address, NX_UINTN Width, NX_U64 *Value);
static NX_UI_STATUS MockMemWrite(NX_U64 Address, NX_UINTN Width, NX_U64 Value);
static const NX_MEMORY_DESCRIPTOR *MockMemGetMemoryMapDescriptors(NX_UINTN *DescriptorCount);
static NX_UI_STATUS MockMemGetMemoryTypeAtAddress(NX_U64 Address, NX_U32 *Type, NX_BOOL *Found);
static void MockMemQueryAccess(NX_U64 BaseAddress, NX_U64 Length, NX_MEMORY_ACCESS *Access);
static void MockMemDumpValidRanges(void);

//
// Number of times DumpValidRanges() was requested.
//
static NX_UINTN gMockDumpCallCount = 0;
static NX_UINTN gMockTypeLookupCount = 0;

//
// Mock memory adapter instance used by host-side integration tests.
//
static MEMORY_ADAPTER_INTERFACE mMemoryAdapterMock = {
    .Init                    = MockMemInit,
    .MemRead                 = MockMemRead,
    .MemWrite                = MockMemWrite,
    .GetMemoryMapDescriptors = MockMemGetMemoryMapDescriptors,
    .GetMemoryTypeAtAddress  = MockMemGetMemoryTypeAtAddress,
    .QueryAccess             = MockMemQueryAccess,
    .DumpValidRanges         = MockMemDumpValidRanges,
};

static
void
MockMemQueryAccess(
    NX_U64           BaseAddress,
    NX_U64           Length,
    NX_MEMORY_ACCESS *Access
    )
{
    if (Access == NULL) {
        return;
    }

    gMockAccessQueryCount++;
    if (Length == 256) {
        gMockPageAccessQueryCount++;
        gMockLastPageAccessBase = BaseAddress;
        gMockLastPageAccessLength = Length;
    }
    gMockLastAccessBase = BaseAddress;
    gMockLastAccessLength = Length;
    Access->QueryStatus = NX_STATUS_NOT_FOUND;
    Access->Attributes = 0;
    Access->ReadAllowed = NX_FALSE;
    Access->WriteAllowed = NX_FALSE;
    Access->ExecuteAllowed = NX_FALSE;
    Access->AccessKnown = NX_FALSE;

    if (Length == 0 || BaseAddress > (~(NX_U64)0) - Length) {
        Access->QueryStatus = NX_STATUS_INVALID_PARAMETER;
        return;
    }

    Access->QueryStatus = NX_STATUS_SUCCESS;
    Access->AccessKnown = NX_TRUE;
    for (NX_UINTN Index = 0; Index < sizeof(gMockDescriptors) / sizeof(gMockDescriptors[0]); Index++) {
        NX_U64 DescriptorLength;
        NX_U64 DescriptorEnd;

        DescriptorLength = gMockDescriptors[Index].NumberOfPages << 12;
        DescriptorEnd = gMockDescriptors[Index].PhysicalStart + DescriptorLength;
        if (BaseAddress >= gMockDescriptors[Index].PhysicalStart &&
            BaseAddress + Length <= DescriptorEnd) {
            *Access = gMockAccess[Index];
            return;
        }
    }
}

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
    gMockReadFailureAddress  = 0;
    gMockReadFailureEnabled  = NX_FALSE;
    for (NX_UINTN Index = 0; Index < sizeof(gMockDescriptors) / sizeof(gMockDescriptors[0]); Index++) {
        gMockAccess[Index].QueryStatus = NX_STATUS_SUCCESS;
        gMockAccess[Index].Attributes = 0;
        gMockAccess[Index].ReadAllowed =
            (gMockDescriptors[Index].Type == NX_MEM_CONVENTIONAL ||
             gMockDescriptors[Index].Type == NX_MEM_LOADER_CODE ||
             gMockDescriptors[Index].Type == NX_MEM_LOADER_DATA ||
             gMockDescriptors[Index].Type == NX_MEM_BOOT_SERVICES_CODE ||
             gMockDescriptors[Index].Type == NX_MEM_BOOT_SERVICES_DATA ||
             gMockDescriptors[Index].Type == NX_MEM_RUNTIME_SERVICES_CODE ||
             gMockDescriptors[Index].Type == NX_MEM_RUNTIME_SERVICES_DATA ||
             gMockDescriptors[Index].Type == NX_MEM_ACPI_RECLAIM ||
             gMockDescriptors[Index].Type == NX_MEM_ACPI_NVS);
        gMockAccess[Index].WriteAllowed =
            (gMockDescriptors[Index].Type == NX_MEM_CONVENTIONAL ||
             gMockDescriptors[Index].Type == NX_MEM_LOADER_CODE ||
             gMockDescriptors[Index].Type == NX_MEM_LOADER_DATA ||
             gMockDescriptors[Index].Type == NX_MEM_BOOT_SERVICES_CODE ||
             gMockDescriptors[Index].Type == NX_MEM_BOOT_SERVICES_DATA ||
             gMockDescriptors[Index].Type == NX_MEM_RUNTIME_SERVICES_CODE ||
             gMockDescriptors[Index].Type == NX_MEM_RUNTIME_SERVICES_DATA);
        gMockAccess[Index].ExecuteAllowed = NX_TRUE;
        gMockAccess[Index].AccessKnown = NX_TRUE;
    }

    return NX_UI_SUCCESS;
}

static
NX_UI_STATUS
MockMemRead(
    NX_U64   Address,
    NX_UINTN Width,
    NX_U64  *Value
    )
{
    NX_UINTN Index;
    NX_MEMORY_ACCESS Access;

    if (Value == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    *Value = 0;

    //
    // Width is ignored by this mock because storage is modeled as 64-bit slots.
    //
    (void)Width;

    MockMemQueryAccess(Address, Width, &Access);
    if (!Access.AccessKnown || !Access.ReadAllowed ||
        !MockMemIsAddressValid(Address) ||
        (gMockReadFailureEnabled && Address == gMockReadFailureAddress)) {
        return NX_UI_ACCESS_DENIED;
    }

    //
    // Map physical address to mock buffer index.
    //
    Index = (Address - 0x1000) / 8;
    if (Index >= 256) {
        return NX_UI_ACCESS_DENIED;
    }

    *Value = gMockMemoryBuffer[Index];
    return NX_UI_SUCCESS;
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
    NX_MEMORY_ACCESS Access;

    //
    // Width is ignored by this mock because storage is modeled as 64-bit slots.
    //
    (void)Width;

    MockMemQueryAccess(Address, Width, &Access);
    if (!Access.AccessKnown || !Access.WriteAllowed ||
        !MockMemIsAddressValid(Address)) {
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
        *DescriptorCount = sizeof(gMockDescriptors) / sizeof(gMockDescriptors[0]);
    }

    return gMockDescriptors;
}

static
NX_UI_STATUS
MockMemGetMemoryTypeAtAddress(
    NX_U64   Address,
    NX_U32  *Type,
    NX_BOOL *Found
    )
{
    if (Type == NULL || Found == NULL) {
        return NX_UI_INVALID_PARAMETER;
    }

    gMockTypeLookupCount++;
    *Type  = 0;
    *Found = NX_FALSE;

    for (NX_UINTN Index = 0; Index < sizeof(gMockDescriptors) / sizeof(gMockDescriptors[0]); Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;
        NX_U64                      Size;

        Desc = &gMockDescriptors[Index];
        Size = Desc->NumberOfPages << 12;
        if (Address >= Desc->PhysicalStart && Address - Desc->PhysicalStart < Size) {
            *Type  = Desc->UefiType;
            *Found = NX_TRUE;
            break;
        }
    }

    return NX_UI_SUCCESS;
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
    NX_U64 Value;

    if (NX_UI_ERROR(MockMemRead(Address, 8, &Value))) {
        return 0;
    }

    return Value;
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

NX_UINTN
MockMemoryGetTypeLookupCount(void)
{
    return gMockTypeLookupCount;
}

NX_UINTN
MockMemoryGetAccessQueryCount(void)
{
    return gMockAccessQueryCount;
}

NX_UINTN
MockMemoryGetPageAccessQueryCount(void)
{
    return gMockPageAccessQueryCount;
}

NX_U64
MockMemoryGetLastAccessBase(void)
{
    return gMockLastAccessBase;
}

NX_U64
MockMemoryGetLastAccessLength(void)
{
    return gMockLastAccessLength;
}

NX_U64
MockMemoryGetLastPageAccessBase(void)
{
    return gMockLastPageAccessBase;
}

NX_U64
MockMemoryGetLastPageAccessLength(void)
{
    return gMockLastPageAccessLength;
}

void
MockMemorySetAccessForDescriptor(
    NX_UINTN Index,
    NX_BOOL  ReadAllowed,
    NX_BOOL  WriteAllowed,
    NX_BOOL  AccessKnown
    )
{
    if (Index >= sizeof(gMockDescriptors) / sizeof(gMockDescriptors[0])) {
        return;
    }

    gMockAccess[Index].QueryStatus = AccessKnown ? NX_STATUS_SUCCESS : NX_STATUS_NOT_FOUND;
    gMockAccess[Index].ReadAllowed = ReadAllowed;
    gMockAccess[Index].WriteAllowed = WriteAllowed;
    gMockAccess[Index].ExecuteAllowed = NX_TRUE;
    gMockAccess[Index].AccessKnown = AccessKnown;
}

/**
  Reset mock memory adapter state.
**/
void
MockMemoryReset(void)
{
    gMockMemoriesInitialized = NX_FALSE;
    gMockDumpCallCount       = 0;
    gMockTypeLookupCount    = 0;
    gMockAccessQueryCount   = 0;
    gMockPageAccessQueryCount = 0;
    gMockLastAccessBase     = 0;
    gMockLastAccessLength   = 0;
    gMockLastPageAccessBase = 0;
    gMockLastPageAccessLength = 0;
    gMockReadFailureAddress = 0;
    gMockReadFailureEnabled = NX_FALSE;
}

/**
  Configure one address to return an explicit read error.
**/
void
MockMemorySetReadFailure(
  NX_U64 Address
  )
{
    gMockReadFailureAddress = Address;
    gMockReadFailureEnabled = NX_TRUE;
}