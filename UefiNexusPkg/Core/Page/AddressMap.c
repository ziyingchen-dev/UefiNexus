/** @file
  Address map implementation for Page Core.

  This module stores memory descriptors provided by the adapter layer and
  provides address, range, and page navigation helpers for PageMem.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../PageCoreTypes.h"
#include "../PageCore.h"

//
// Cached memory descriptor list provided by the adapter layer.
//
// Core does not own this buffer. The caller must ensure the descriptor buffer
// remains valid after PageCoreInitAddressMap() succeeds.
//
static const NX_MEMORY_DESCRIPTOR *gMemoryDescriptors = NULL;
static NX_UINTN                    gDescriptorCount   = 0;

NX_STATUS
PageCoreInitAddressMap(
    const NX_MEMORY_DESCRIPTOR *Descriptors,
    NX_UINTN                    DescriptorCount
    )
{
    if (Descriptors == NULL || DescriptorCount == 0) {
        return NX_STATUS_INVALID_PARAMETER;
    }

    gMemoryDescriptors = Descriptors;
    gDescriptorCount   = DescriptorCount;

    return NX_STATUS_SUCCESS;
}

/**
  Check whether a descriptor type is valid for PageMem browsing.
**/
static
NX_BOOL
IsDescriptorValid(
    const NX_MEMORY_DESCRIPTOR *Desc
    )
{
    if (Desc == NULL) {
        return NX_FALSE;
    }

    switch (Desc->Type) {
    case NX_MEM_LOADER_CODE:
    case NX_MEM_LOADER_DATA:
    case NX_MEM_CONVENTIONAL:
    case NX_MEM_BOOT_SERVICES_CODE:
    case NX_MEM_BOOT_SERVICES_DATA:
    case NX_MEM_RUNTIME_SERVICES_CODE:
    case NX_MEM_RUNTIME_SERVICES_DATA:
    case NX_MEM_ACPI_RECLAIM:
    case NX_MEM_ACPI_NVS:
    case NX_MEM_MMIO:
        return NX_TRUE;
    default:
        return NX_FALSE;
    }
}

NX_BOOL
PageCoreIsAddressValid(
    NX_U64 Address
    )
{
    NX_UINTN Index;

    if (gMemoryDescriptors == NULL) {
        return NX_FALSE;
    }

    for (Index = 0; Index < gDescriptorCount; Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;
        NX_U64                      DescStart;
        NX_U64                      DescEnd;

        Desc = &gMemoryDescriptors[Index];

        if (!IsDescriptorValid(Desc)) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd   = Desc->PhysicalStart + (Desc->NumberOfPages << 12);

        if (Address >= DescStart && Address < DescEnd) {
            return NX_TRUE;
        }
    }

    return NX_FALSE;
}

NX_BOOL
PageCoreIsRangeValid(
    NX_U64   Address,
    NX_UINTN Size
    )
{
    NX_UINTN Index;
    NX_U64   RangeEnd;

    if (gMemoryDescriptors == NULL || Size == 0) {
        return NX_FALSE;
    }

    RangeEnd = Address + Size;

    for (Index = 0; Index < gDescriptorCount; Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;
        NX_U64                      DescStart;
        NX_U64                      DescEnd;

        Desc = &gMemoryDescriptors[Index];

        if (!IsDescriptorValid(Desc)) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd   = Desc->PhysicalStart + (Desc->NumberOfPages << 12);

        if (Address >= DescStart && RangeEnd <= DescEnd) {
            return NX_TRUE;
        }
    }

    return NX_FALSE;
}

NX_U64
PageCoreGetFirstValidAddress(
    void
    )
{
    NX_UINTN Index;

    if (gMemoryDescriptors == NULL) {
        return 0;
    }

    for (Index = 0; Index < gDescriptorCount; Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;

        Desc = &gMemoryDescriptors[Index];

        if (IsDescriptorValid(Desc)) {
            return Desc->PhysicalStart;
        }
    }

    return 0;
}

/**
  Align an address up to the requested alignment.

  The caller is responsible for passing a non-zero power-of-two alignment.
**/
static
NX_U64
AlignUp(
    NX_U64   Address,
    NX_UINTN Alignment
    )
{
    if (Alignment == 0) {
        return Address;
    }

    return (Address + (Alignment - 1)) & ~((NX_U64)(Alignment - 1));
}

/**
  Check whether a full page is inside one valid descriptor.
**/
static
NX_BOOL
IsPageValid(
    NX_U64   Address,
    NX_UINTN PageSize
    )
{
    NX_U64   PageBase;
    NX_U64   PageEnd;
    NX_UINTN Index;

    if (PageSize == 0) {
        return NX_FALSE;
    }

    PageBase = Address & ~((NX_U64)(PageSize - 1));
    PageEnd  = PageBase + PageSize;

    for (Index = 0; Index < gDescriptorCount; Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;
        NX_U64                      DescStart;
        NX_U64                      DescEnd;

        Desc = &gMemoryDescriptors[Index];

        if (!IsDescriptorValid(Desc) || Desc->NumberOfPages == 0) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd   = Desc->PhysicalStart + (Desc->NumberOfPages << 12);

        if (PageBase >= DescStart && PageEnd <= DescEnd) {
            return NX_TRUE;
        }
    }

    return NX_FALSE;
}

NX_U64
PageCoreGetNextValidPageAddress(
    NX_U64   Address,
    NX_UINTN PageSize
    )
{
    NX_U64   CurrentPage;
    NX_U64   NextPage;
    NX_U64   BestPage;
    NX_UINTN Index;

    if (gMemoryDescriptors == NULL || PageSize == 0) {
        return Address;
    }

    CurrentPage = Address & ~((NX_U64)(PageSize - 1));
    if (CurrentPage > (NX_U64)~(NX_U64)0 - PageSize) {
        return CurrentPage;
    }

    NextPage = CurrentPage + PageSize;
    if (IsPageValid(NextPage, PageSize)) {
        return NextPage;
    }

    BestPage = CurrentPage;

    for (Index = 0; Index < gDescriptorCount; Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;
        NX_U64                      DescStart;
        NX_U64                      DescEnd;
        NX_U64                      FirstPage;
        NX_U64                      Candidate;

        Desc = &gMemoryDescriptors[Index];

        if (!IsDescriptorValid(Desc) || Desc->NumberOfPages == 0) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd   = Desc->PhysicalStart + (Desc->NumberOfPages << 12);
        FirstPage = AlignUp(DescStart, PageSize);

        if (DescEnd < PageSize || FirstPage + PageSize > DescEnd) {
            continue;
        }

        if (NextPage <= FirstPage) {
            Candidate = FirstPage;
        } else if (NextPage < DescEnd) {
            Candidate = AlignUp(NextPage, PageSize);
            if (Candidate + PageSize > DescEnd) {
                continue;
            }
        } else {
            continue;
        }

        if (Candidate > CurrentPage &&
            (BestPage == CurrentPage || Candidate < BestPage)) {
            BestPage = Candidate;
        }
    }

    return BestPage;
}

NX_U64
PageCoreGetPreviousValidPageAddress(
    NX_U64   Address,
    NX_UINTN PageSize
    )
{
    NX_U64   CurrentPage;
    NX_U64   PrevPage;
    NX_U64   BestPage;
    NX_UINTN Index;

    if (gMemoryDescriptors == NULL || PageSize == 0 || Address == 0) {
        return Address;
    }

    CurrentPage = Address & ~((NX_U64)(PageSize - 1));
    if (CurrentPage < PageSize) {
        return CurrentPage;
    }

    PrevPage = CurrentPage - PageSize;
    if (IsPageValid(PrevPage, PageSize)) {
        return PrevPage;
    }

    BestPage = 0;

    for (Index = 0; Index < gDescriptorCount; Index++) {
        const NX_MEMORY_DESCRIPTOR *Desc;
        NX_U64                      DescStart;
        NX_U64                      DescEnd;
        NX_U64                      FirstPage;
        NX_U64                      LastPage;
        NX_U64                      Candidate;

        Desc = &gMemoryDescriptors[Index];

        if (!IsDescriptorValid(Desc) || Desc->NumberOfPages == 0) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd   = Desc->PhysicalStart + (Desc->NumberOfPages << 12);
        FirstPage = AlignUp(DescStart, PageSize);

        if (DescEnd < PageSize || FirstPage + PageSize > DescEnd) {
            continue;
        }

        LastPage = (DescEnd - PageSize) & ~((NX_U64)(PageSize - 1));
        if (LastPage < FirstPage) {
            continue;
        }

        if (CurrentPage <= FirstPage) {
            continue;
        }

        if (CurrentPage > LastPage) {
            Candidate = LastPage;
        } else {
            Candidate = CurrentPage - PageSize;
            if (Candidate < FirstPage) {
                continue;
            }
        }

        if (Candidate < CurrentPage && Candidate > BestPage) {
            BestPage = Candidate;
        }
    }

    return BestPage ? BestPage : CurrentPage;
}