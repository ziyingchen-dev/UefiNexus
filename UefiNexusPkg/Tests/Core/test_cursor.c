/** @file
  Unit tests for Page Core cursor engine.

  This module validates PageMem cursor movement, width-aware cursor behavior,
  boundary clamping, page alignment, and current address calculation. These
  tests are host-side Core-only tests and do not depend on UEFI services or
  adapters.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>

#include "../../UI/Pages/PageMem/PageMemCore.h"
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
// Validate basic cursor movement with 1-byte width.
//
static
int
TestCursorMovement(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, 0, 1 };

    PageCoreMoveCursor(&State, 1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 1, "Cursor forward 1 byte");

    PageCoreMoveCursor(&State, 15, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 16, "Cursor forward 16 bytes total");

    PageCoreMoveCursor(&State, -8, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 8, "Cursor backward 8 bytes");

    return 1;
}

//
// Validate cursor movement scales by selected access width.
//
static
int
TestCursorMovementByWidth(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, 0, 4 };

    PageCoreMoveCursor(&State, 1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 4, "Cursor moves one 4-byte cell");

    PageCoreMoveCursor(&State, -1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 0, "Cursor moves back one 4-byte cell");

    State.Offset = 6;
    PageCoreMoveCursor(&State, 0, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 4, "Cursor realigns to width boundary");

    return 1;
}

//
// Validate cursor movement is clamped to page boundaries.
//
static
int
TestCursorBoundaries(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, PAGE_SIZE - 1, 1 };

    PageCoreMoveCursor(&State, 1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, PAGE_SIZE - 1, "Cursor at end of page boundary");

    State.Offset = 0;
    PageCoreMoveCursor(&State, -1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 0, "Cursor at start of page boundary");

    return 1;
}

//
// Validate cursor width changes realign the current offset.
//
static
int
TestCursorWidthChange(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, 6, 1 };

    PageCoreSetCursorWidth(&State, 4);
    ASSERT_EQ(State.Width, 4, "Width change updates cursor width");
    ASSERT_EQ(State.Offset, 4, "Width change realigns offset");

    return 1;
}

//
// Validate address alignment to PageMem page boundary.
//
static
int
TestAlignToPage(void)
{
    ASSERT_EQ(PageCoreAlignToPage(0x1234, PAGE_SIZE), 0x1200, "Align 0x1234 to page");
    ASSERT_EQ(PageCoreAlignToPage(0x1200, PAGE_SIZE), 0x1200, "Already aligned address");
    ASSERT_EQ(PageCoreAlignToPage(0, PAGE_SIZE), 0, "Align zero address");

    return 1;
}

//
// Validate absolute address calculation from page base and cursor offset.
//
static
int
TestCurrentAddress(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x2000, 100, 1 };

    ASSERT_EQ(PageCoreGetCurrentAddress(&State), 0x2064, "Current address calculation");

    return 1;
}

/**
  Run all Page Core cursor engine unit tests.
**/
void
RunCursorTests(void)
{
    printf("\n=== Cursor Engine Tests ===\n");

    TEST(TestCursorMovement, "Cursor Movement");
    TEST(TestCursorMovementByWidth, "Cursor Movement By Width");
    TEST(TestCursorBoundaries, "Cursor Boundaries");
    TEST(TestCursorWidthChange, "Cursor Width Change");
    TEST(TestAlignToPage, "Align To Page");
    TEST(TestCurrentAddress, "Current Address");

    printf("\n=== Cursor Engine Tests Complete ===\n");
}