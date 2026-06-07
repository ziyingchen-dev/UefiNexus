/** @file
  Unit tests for Page Core layout engine.

  This module validates row, column, and screen position calculations used by
  PageMem rendering. These tests are host-side Core-only tests and do not depend
  on UEFI services or adapters.

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
// Validate row calculation from page offset.
//
static
int
TestCellRow(void)
{
    ASSERT_EQ(PageCoreGetCellRow(0, COLS), 0, "Cell row for offset 0");
    ASSERT_EQ(PageCoreGetCellRow(COLS, COLS), 1, "Cell row for offset COLS");
    ASSERT_EQ(PageCoreGetCellRow(PAGE_SIZE - 1, COLS), PAGE_SIZE / COLS - 1, "Cell row for last byte");

    return 1;
}

//
// Validate column calculation from page offset.
//
static
int
TestCellCol(void)
{
    ASSERT_EQ(PageCoreGetCellCol(0, COLS), 0, "Cell col for offset 0");
    ASSERT_EQ(PageCoreGetCellCol(1, COLS), 1, "Cell col for offset 1");
    ASSERT_EQ(PageCoreGetCellCol(COLS, COLS), 0, "Cell col for offset COLS");

    return 1;
}

//
// Validate full screen position calculation from page offset.
//
static
int
TestScreenPosition(void)
{
    NX_UINTN Row;
    NX_UINTN Col;

    PageCoreGetScreenPosition(0, COLS, &Row, &Col);
    ASSERT_EQ(Row, PAGEMEM_DATA_ROW, "Screen row for first cell");
    ASSERT_EQ(Col, 0, "Screen col for first cell");

    PageCoreGetScreenPosition(COLS, COLS, &Row, &Col);
    ASSERT_EQ(Row, PAGEMEM_DATA_ROW + 1, "Screen row for first cell of second row");
    ASSERT_EQ(Col, 0, "Screen col for first cell of second row");

    return 1;
}

/**
  Run all Page Core layout engine unit tests.
**/
void
RunLayoutTests(void)
{
    printf("\n=== Layout Engine Tests ===\n");

    TEST(TestCellRow, "Cell Row Calculation");
    TEST(TestCellCol, "Cell Column Calculation");
    TEST(TestScreenPosition, "Screen Position Calculation");

    printf("\n=== Layout Engine Tests Complete ===\n");
}