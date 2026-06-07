/** @file
  Unit tests for Page Core address map.

  This module validates address map initialization, address validation, range
  validation, and valid page navigation. These tests are host-side Core-only
  tests and do not depend on UEFI services or adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>

#include "../../Core/PageCore.h"
#include "test_common.h"

//
// Assertion helper for integer equality.
//
#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

//
// Assertion helper for boolean conditions.
//
#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

//
// Test descriptor set used by address map unit tests.
//
static const NX_MEMORY_DESCRIPTOR gDescriptors[] = {
    { NX_MEM_OTHER,        0x0000, 1, 0 },
    { NX_MEM_CONVENTIONAL, 0x1000, 2, 0 },
    { NX_MEM_LOADER_CODE,  0x4000, 1, 0 },
};

//
// Validate address map initialization and first valid address lookup.
//
static
int
TestInitAndFirstAddress(void)
{
    ASSERT_EQ(PageCoreInitAddressMap(gDescriptors, 3), NX_STATUS_SUCCESS, "Init address map");
    ASSERT_EQ(PageCoreGetFirstValidAddress(), 0x1000, "First valid address");

    return 1;
}

//
// Validate single-address lookup against valid and invalid descriptors.
//
static
int
TestAddressValidation(void)
{
    PageCoreInitAddressMap(gDescriptors, 3);

    ASSERT_TRUE(PageCoreIsAddressValid(0x1000) == NX_TRUE, "Valid conventional address");
    ASSERT_TRUE(PageCoreIsAddressValid(0x1FFF) == NX_TRUE, "Valid address inside descriptor");
    ASSERT_TRUE(PageCoreIsAddressValid(0x0500) == NX_FALSE, "Invalid reserved address");

    return 1;
}

//
// Validate range lookup and boundary crossing behavior.
//
static
int
TestRangeValidation(void)
{
    PageCoreInitAddressMap(gDescriptors, 3);

    ASSERT_TRUE(PageCoreIsRangeValid(0x1000, 0x100) == NX_TRUE, "Valid range");
    ASSERT_TRUE(PageCoreIsRangeValid(0x2F80, 0x200) == NX_FALSE, "Range crossing boundary");

    return 1;
}

//
// Validate next/previous valid page navigation across descriptors.
//
static
int
TestPageNavigation(void)
{
    PageCoreInitAddressMap(gDescriptors, 3);

    ASSERT_EQ(PageCoreGetNextValidPageAddress(0x1000, PAGE_SIZE), 0x1100, "Next valid page");
    ASSERT_EQ(PageCoreGetPreviousValidPageAddress(0x1100, PAGE_SIZE), 0x1000, "Previous valid page");
    ASSERT_EQ(PageCoreGetNextValidPageAddress(0x2F00, PAGE_SIZE), 0x4000, "Skip to next valid descriptor");

    return 1;
}

/**
  Run all Page Core address map unit tests.
**/
void
RunAddressMapTests(void)
{
    printf("\n=== Address Map Tests ===\n");

    TEST(TestInitAndFirstAddress, "Init And First Address");
    TEST(TestAddressValidation, "Address Validation");
    TEST(TestRangeValidation, "Range Validation");
    TEST(TestPageNavigation, "Page Navigation");

    printf("\n=== Address Map Tests Complete ===\n");
}