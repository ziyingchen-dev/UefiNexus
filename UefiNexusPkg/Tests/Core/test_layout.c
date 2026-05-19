/**
 * test_layout.c - Unit tests for Layout Engine
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

#define TEST(func, name) \
    do { \
        if (!(func)()) { \
            printf("\nTest %s FAILED\n", name); \
        } else { \
            printf("\nTest %s PASSED\n", name); \
        } \
    } while (0)

static int
TestCellRow(void)
{
    ASSERT_EQ(PageMemCoreGetCellRow(0, COLS), 0, "Cell row for offset 0");
    ASSERT_EQ(PageMemCoreGetCellRow(COLS, COLS), 1, "Cell row for offset COLS");
    ASSERT_EQ(PageMemCoreGetCellRow(PAGE_SIZE - 1, COLS), PAGE_SIZE / COLS - 1, "Cell row for last byte");
    return 1;
}

static int
TestCellCol(void)
{
    ASSERT_EQ(PageMemCoreGetCellCol(0, COLS), 0, "Cell col for offset 0");
    ASSERT_EQ(PageMemCoreGetCellCol(1, COLS), 1, "Cell col for offset 1");
    ASSERT_EQ(PageMemCoreGetCellCol(COLS, COLS), 0, "Cell col for offset COLS");
    return 1;
}

static int
TestScreenPosition(void)
{
    PM_UINTN Row;
    PM_UINTN Col;

    PageMemCoreGetScreenPosition(0, COLS, &Row, &Col);
    ASSERT_EQ(Row, PAGEMEM_DATA_ROW, "Screen row for first cell");
    ASSERT_EQ(Col, 0, "Screen col for first cell");

    PageMemCoreGetScreenPosition(COLS, COLS, &Row, &Col);
    ASSERT_EQ(Row, PAGEMEM_DATA_ROW + 1, "Screen row for first cell of second row");
    ASSERT_EQ(Col, 0, "Screen col for first cell of second row");

    return 1;
}

void
RunLayoutTests(void)
{
    printf("\n=== Layout Engine Tests ===\n");

    TEST(TestCellRow, "Cell Row Calculation");
    TEST(TestCellCol, "Cell Column Calculation");
    TEST(TestScreenPosition, "Screen Position Calculation");

    printf("\n=== Layout Engine Tests Complete ===\n");
}
