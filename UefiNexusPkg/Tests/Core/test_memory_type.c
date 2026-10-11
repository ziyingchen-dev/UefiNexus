/** @file
  Host-side tests for shared UEFI memory type display mapping.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>
#include <string.h>

#include "../../Core/UefiMemoryType.h"
#include "test_common.h"

typedef struct {
    NX_U32       Type;
    const NX_CHAR *Name;
} MEMORY_TYPE_CASE;

static const MEMORY_TYPE_CASE gKnownTypeCases[] = {
    { 0,  "EfiReservedMemoryType" },
    { 1,  "EfiLoaderCode" },
    { 2,  "EfiLoaderData" },
    { 3,  "EfiBootServicesCode" },
    { 4,  "EfiBootServicesData" },
    { 5,  "EfiRuntimeServicesCode" },
    { 6,  "EfiRuntimeServicesData" },
    { 7,  "EfiConventionalMemory" },
    { 8,  "EfiUnusableMemory" },
    { 9,  "EfiACPIReclaimMemory" },
    { 10, "EfiACPIMemoryNVS" },
    { 11, "EfiMemoryMappedIO" },
    { 12, "EfiMemoryMappedIOPortSpace" },
    { 13, "EfiPalCode" }
};

#define ASSERT_TRUE(condition, test_name) \
    do { \
        if (!(condition)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return NX_FALSE; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

static
NX_BOOL
TestKnownMemoryTypeMappings(void)
{
    NX_CHAR Name[32];

    for (NX_UINTN Index = 0; Index < sizeof(gKnownTypeCases) / sizeof(gKnownTypeCases[0]); Index++) {
        UefiMemoryTypeFormat(
            gKnownTypeCases[Index].Type,
            Name,
            sizeof(Name)
            );

        if (strcmp(Name, gKnownTypeCases[Index].Name) != 0) {
            printf("[FAIL] Known memory type mapping for %u\n", gKnownTypeCases[Index].Type);
            return NX_FALSE;
        }
    }

    printf("[PASS] Known memory type names\n");
    return NX_TRUE;
}

static
NX_BOOL
TestUnknownMemoryTypeRetainsNumericValue(void)
{
    NX_CHAR Name[32];

    UefiMemoryTypeFormat(14, Name, sizeof(Name));
    ASSERT_TRUE(strcmp(Name, "Unknown(14)") == 0, "Unknown type retains numeric value");

    UefiMemoryTypeFormat(0xFFFFFFFFU, Name, sizeof(Name));
    ASSERT_TRUE(strcmp(Name, "Unknown(4294967295)") == 0,
                "Large unknown type retains numeric value");

    return NX_TRUE;
}

void
RunMemoryTypeTests(void)
{
    printf("\n=== Shared Memory Type Tests ===\n");
    TEST(TestKnownMemoryTypeMappings, "Known Memory Type Mappings");
    TEST(TestUnknownMemoryTypeRetainsNumericValue, "Unknown Memory Type Numeric Value");
    printf("\n=== Shared Memory Type Tests Complete ===\n");
}
