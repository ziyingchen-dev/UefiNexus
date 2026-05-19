/**
 * AddressMap.c - Manages valid memory address ranges
 * Depends on adapter for initial memory descriptor data
 */

#include "../PageMemCoreTypes.h"
#include "../PageMemCore.h"

// Simple address range validation
// In production, this would maintain a list of valid ranges
// For now, using a simplified approach

static const PM_MEMORY_DESCRIPTOR *gMemoryDescriptors = NULL;
static PM_UINTN gDescriptorCount = 0;

/**
 * Initialize address map from descriptors
 */
PM_STATUS
PageMemCoreInitAddressMap(
    const PM_MEMORY_DESCRIPTOR *Descriptors,
    PM_UINTN DescriptorCount
    )
{
    if (Descriptors == NULL || DescriptorCount == 0) {
        return PM_STATUS_INVALID_PARAMETER;
    }

    gMemoryDescriptors = Descriptors;
    gDescriptorCount = DescriptorCount;

    return PM_STATUS_SUCCESS;
}

/**
 * Check if a descriptor matches the address
 */
static PM_BOOL
IsDescriptorValid(const PM_MEMORY_DESCRIPTOR *Desc)
{
    if (Desc == NULL) {
        return PM_FALSE;
    }

    switch (Desc->Type) {
    case PM_MEM_LOADER_CODE:
    case PM_MEM_LOADER_DATA:
    case PM_MEM_CONVENTIONAL:
    case PM_MEM_BOOT_SERVICES_CODE:
    case PM_MEM_BOOT_SERVICES_DATA:
    case PM_MEM_RUNTIME_SERVICES_CODE:
    case PM_MEM_RUNTIME_SERVICES_DATA:
        return PM_TRUE;
    default:
        return PM_FALSE;
    }
}

/**
 * Check if address is valid
 */
PM_BOOL
PageMemCoreIsAddressValid(
    PM_U64 Address
    )
{
    PM_UINTN i;

    if (gMemoryDescriptors == NULL) {
        return PM_FALSE;
    }

    for (i = 0; i < gDescriptorCount; i++) {
        const PM_MEMORY_DESCRIPTOR *Desc = &gMemoryDescriptors[i];
        PM_U64 DescStart;
        PM_U64 DescEnd;

        if (!IsDescriptorValid(Desc)) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd = Desc->PhysicalStart + (Desc->NumberOfPages << 12);

        if (Address >= DescStart && Address < DescEnd) {
            return PM_TRUE;
        }
    }

    return PM_FALSE;
}

/**
 * Check if range is valid
 */
PM_BOOL
PageMemCoreIsRangeValid(
    PM_U64 Address,
    PM_UINTN Size
    )
{
    PM_UINTN i;
    PM_U64 RangeEnd;

    if (gMemoryDescriptors == NULL || Size == 0) {
        return PM_FALSE;
    }

    RangeEnd = Address + Size;

    for (i = 0; i < gDescriptorCount; i++) {
        const PM_MEMORY_DESCRIPTOR *Desc = &gMemoryDescriptors[i];
        PM_U64 DescStart;
        PM_U64 DescEnd;

        if (!IsDescriptorValid(Desc)) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd = Desc->PhysicalStart + (Desc->NumberOfPages << 12);
        if (Address >= DescStart && RangeEnd <= DescEnd) {
            return PM_TRUE;
        }
    }

    return PM_FALSE;
}

/**
 * Get first valid address
 */
PM_U64
PageMemCoreGetFirstValidAddress(void)
{
    PM_UINTN i;

    if (gMemoryDescriptors == NULL) {
        return 0;
    }

    for (i = 0; i < gDescriptorCount; i++) {
        const PM_MEMORY_DESCRIPTOR *Desc = &gMemoryDescriptors[i];

        if (IsDescriptorValid(Desc)) {
            return Desc->PhysicalStart;
        }
    }

    return 0;
}

/**
 * Align address up to the nearest page boundary.
 */
static PM_U64
AlignUp(
    PM_U64 Address,
    PM_UINTN Alignment
    )
{
    if (Alignment == 0) {
        return Address;
    }

    return (Address + (Alignment - 1)) & ~((PM_U64)(Alignment - 1));
}

/**
 * Check if a page is valid (all addresses in the page are accessible)
 */
static PM_BOOL
IsPageValid(PM_U64 Address, PM_UINTN PageSize)
{
    PM_U64 PageBase;
    PM_U64 PageEnd;
    PM_UINTN i;

    if (PageSize == 0) {
        return PM_FALSE;
    }

    PageBase = Address & ~((PM_U64)(PageSize - 1));
    PageEnd  = PageBase + PageSize;

    for (i = 0; i < gDescriptorCount; i++) {
        const PM_MEMORY_DESCRIPTOR *Desc = &gMemoryDescriptors[i];
        PM_U64 DescStart;
        PM_U64 DescEnd;

        if (!IsDescriptorValid(Desc) || Desc->NumberOfPages == 0) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd = Desc->PhysicalStart + (Desc->NumberOfPages << 12);

        if (PageBase >= DescStart && PageEnd <= DescEnd) {
            return PM_TRUE;
        }
    }

    return PM_FALSE;
}

/**
 * Get next valid page address
 */
PM_U64
PageMemCoreGetNextValidPageAddress(
    PM_U64 Address,
    PM_UINTN PageSize
    )
{
    PM_U64 CurrentPage;
    PM_U64 NextPage;
    PM_U64 BestPage;
    PM_UINTN i;

    if (gMemoryDescriptors == NULL || PageSize == 0) {
        return Address;
    }

    CurrentPage = Address & ~((PM_U64)(PageSize - 1));
    if (CurrentPage > (PM_U64)~(PM_U64)0 - PageSize) {
        return CurrentPage;
    }

    NextPage = CurrentPage + PageSize;
    if (IsPageValid(NextPage, PageSize)) {
        return NextPage;
    }

    BestPage = CurrentPage;

    for (i = 0; i < gDescriptorCount; i++) {
        const PM_MEMORY_DESCRIPTOR *Desc = &gMemoryDescriptors[i];
        PM_U64 DescStart;
        PM_U64 DescEnd;
        PM_U64 FirstPage;
        PM_U64 Candidate;

        if (!IsDescriptorValid(Desc) || Desc->NumberOfPages == 0) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd = Desc->PhysicalStart + (Desc->NumberOfPages << 12);
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

        if (Candidate > CurrentPage && (BestPage == CurrentPage || Candidate < BestPage)) {
            BestPage = Candidate;
        }
    }

    return BestPage;
}

/**
 * Get previous valid page address
 */
PM_U64
PageMemCoreGetPreviousValidPageAddress(
    PM_U64 Address,
    PM_UINTN PageSize
    )
{
    PM_U64 CurrentPage;
    PM_U64 PrevPage;
    PM_U64 BestPage;
    PM_UINTN i;

    if (gMemoryDescriptors == NULL || PageSize == 0 || Address == 0) {
        return Address;
    }

    CurrentPage = Address & ~((PM_U64)(PageSize - 1));
    if (CurrentPage < PageSize) {
        return CurrentPage;
    }

    PrevPage = CurrentPage - PageSize;
    if (IsPageValid(PrevPage, PageSize)) {
        return PrevPage;
    }

    BestPage = 0;

    for (i = 0; i < gDescriptorCount; i++) {
        const PM_MEMORY_DESCRIPTOR *Desc = &gMemoryDescriptors[i];
        PM_U64 DescStart;
        PM_U64 DescEnd;
        PM_U64 FirstPage;
        PM_U64 LastPage;
        PM_U64 Candidate;

        if (!IsDescriptorValid(Desc) || Desc->NumberOfPages == 0) {
            continue;
        }

        DescStart = Desc->PhysicalStart;
        DescEnd = Desc->PhysicalStart + (Desc->NumberOfPages << 12);
        FirstPage = AlignUp(DescStart, PageSize);
        if (DescEnd < PageSize || FirstPage + PageSize > DescEnd) {
            continue;
        }

        LastPage = (DescEnd - PageSize) & ~((PM_U64)(PageSize - 1));
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
