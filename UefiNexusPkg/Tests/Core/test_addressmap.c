/**
 * test_addressmap.c - Unit tests for AddressMap
 */

#include <stdio.h>
#include "../../Core/PageMemCore.h"

#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return 0; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define TEST(func, name) \
    do { \
        if (!(func)()) { \
            printf("\nTest %s FAILED\n", name); \
        } else { \
            printf("\nTest %s PASSED\n", name); \
        } \
    } while (0)

static const PM_MEMORY_DESCRIPTOR gDescriptors[] = {
    { PM_MEM_OTHER, 0x0000, 1, 0 },
    { PM_MEM_CONVENTIONAL, 0x1000, 2, 0 },
    { PM_MEM_LOADER_CODE, 0x4000, 1, 0 },
};

static int
TestInitAndFirstAddress(void)
{
    ASSERT_EQ(PageMemCoreInitAddressMap(gDescriptors, 3), PM_STATUS_SUCCESS, "Init address map");
    ASSERT_EQ(PageMemCoreGetFirstValidAddress(), 0x1000, "First valid address");
    return 1;
}

static int
TestAddressValidation(void)
{
    PageMemCoreInitAddressMap(gDescriptors, 3);
    ASSERT_TRUE(PageMemCoreIsAddressValid(0x1000) == PM_TRUE, "Valid conventional address");
    ASSERT_TRUE(PageMemCoreIsAddressValid(0x1FFF) == PM_TRUE, "Valid address inside descriptor");
    ASSERT_TRUE(PageMemCoreIsAddressValid(0x0500) == PM_FALSE, "Invalid reserved address");
    return 1;
}

static int
TestRangeValidation(void)
{
    PageMemCoreInitAddressMap(gDescriptors, 3);
    ASSERT_TRUE(PageMemCoreIsRangeValid(0x1000, 0x100) == PM_TRUE, "Valid range");
    ASSERT_TRUE(PageMemCoreIsRangeValid(0x2F80, 0x200) == PM_FALSE, "Range crossing boundary");
    return 1;
}

static int
TestPageNavigation(void)
{
    PageMemCoreInitAddressMap(gDescriptors, 3);
    ASSERT_EQ(PageMemCoreGetNextValidPageAddress(0x1000, PAGE_SIZE), 0x1100, "Next valid page");
    ASSERT_EQ(PageMemCoreGetPreviousValidPageAddress(0x1100, PAGE_SIZE), 0x1000, "Previous valid page");
    ASSERT_EQ(PageMemCoreGetNextValidPageAddress(0x2F00, PAGE_SIZE), 0x4000, "Skip to next valid descriptor");
    return 1;
}

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
