/** @file
  Mock EFI_MEMORY_ATTRIBUTE_PROTOCOL for host-side tests.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Protocol/MemoryAttribute.h>

#define MOCK_ATTRIBUTE_RANGE_COUNT 16

typedef struct {
    UINT64 BaseAddress;
    UINT64 Length;
    UINT64 Attributes;
} MOCK_ATTRIBUTE_RANGE;

EFI_GUID gEfiMemoryAttributeProtocolGuid = EFI_MEMORY_ATTRIBUTE_PROTOCOL_GUID;

static MOCK_ATTRIBUTE_RANGE gRanges[MOCK_ATTRIBUTE_RANGE_COUNT];
static UINTN                gRangeCount = 0;
static EFI_STATUS           gMockStatus = EFI_SUCCESS;
static UINTN                gQueryCount = 0;

static
EFI_STATUS
EFIAPI
MockGetMemoryAttributes(
    EFI_MEMORY_ATTRIBUTE_PROTOCOL *This,
    UINT64                         BaseAddress,
    UINT64                         Length,
    UINT64                        *Attributes
    )
{
    UINT64 RangeEnd;

    (void)This;
    gQueryCount++;

    if (Length == 0 || Attributes == NULL ||
        BaseAddress > MAX_UINT64 - Length) {
        return EFI_INVALID_PARAMETER;
    }

    if (EFI_ERROR(gMockStatus)) {
        return gMockStatus;
    }

    RangeEnd = BaseAddress + Length;
    for (UINTN Index = 0; Index < gRangeCount; Index++) {
        UINT64 RuleEnd;

        if (gRanges[Index].Length == 0 ||
            gRanges[Index].BaseAddress > MAX_UINT64 - gRanges[Index].Length) {
            continue;
        }

        RuleEnd = gRanges[Index].BaseAddress + gRanges[Index].Length;
        if (BaseAddress >= gRanges[Index].BaseAddress && RangeEnd <= RuleEnd) {
            *Attributes = gRanges[Index].Attributes;
            return EFI_SUCCESS;
        }
    }

    return EFI_NO_MAPPING;
}

static EFI_MEMORY_ATTRIBUTE_PROTOCOL gProtocol = {
    MockGetMemoryAttributes,
    NULL,
    NULL
};

void
MemoryAttributeMockReset(void)
{
    gRangeCount = 0;
    gMockStatus = EFI_SUCCESS;
    gQueryCount = 0;
}

void
MemoryAttributeMockAddRange(
    UINT64 BaseAddress,
    UINT64 Length,
    UINT64 Attributes
    )
{
    if (gRangeCount < MOCK_ATTRIBUTE_RANGE_COUNT) {
        gRanges[gRangeCount].BaseAddress = BaseAddress;
        gRanges[gRangeCount].Length = Length;
        gRanges[gRangeCount].Attributes = Attributes;
        gRangeCount++;
    }
}

void
MemoryAttributeMockSetStatus(
    EFI_STATUS Status
    )
{
    gMockStatus = Status;
}

UINTN
MemoryAttributeMockGetQueryCount(void)
{
    return gQueryCount;
}

EFI_MEMORY_ATTRIBUTE_PROTOCOL *
MemoryAttributeMockGetProtocol(void)
{
    return &gProtocol;
}
