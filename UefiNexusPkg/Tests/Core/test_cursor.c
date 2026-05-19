/**
 * test_cursor.c - Unit tests for Cursor Engine
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
TestCursorMovement(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, 0, 1 };

    PageMemCoreMoveCursor(&State, 1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 1, "Cursor forward 1 byte");

    PageMemCoreMoveCursor(&State, 15, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 16, "Cursor forward 16 bytes total");

    PageMemCoreMoveCursor(&State, -8, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 8, "Cursor backward 8 bytes");

    return 1;
}

static int
TestCursorMovementByWidth(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, 0, 4 };

    PageMemCoreMoveCursor(&State, 1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 4, "Cursor moves one 4-byte cell");

    PageMemCoreMoveCursor(&State, -1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 0, "Cursor moves back one 4-byte cell");

    State.Offset = 6;
    PageMemCoreMoveCursor(&State, 0, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 4, "Cursor realigns to width boundary");

    return 1;
}

static int
TestCursorBoundaries(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x1000, PAGE_SIZE - 1, 1 };

    PageMemCoreMoveCursor(&State, 1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, PAGE_SIZE - 1, "Cursor at end of page boundary");

    State.Offset = 0;
    PageMemCoreMoveCursor(&State, -1, PAGE_SIZE);
    ASSERT_EQ(State.Offset, 0, "Cursor at start of page boundary");

    return 1;
}

static int
TestAlignToPage(void)
{
    ASSERT_EQ(PageMemCoreAlignToPage(0x1234, PAGE_SIZE), 0x1200, "Align 0x1234 to page");
    ASSERT_EQ(PageMemCoreAlignToPage(0x1200, PAGE_SIZE), 0x1200, "Already aligned address");
    ASSERT_EQ(PageMemCoreAlignToPage(0, PAGE_SIZE), 0, "Align zero address");

    return 1;
}

static int
TestCurrentAddress(void)
{
    PAGEMEM_CURSOR_STATE State = { 0x2000, 100, 1 };

    ASSERT_EQ(PageMemCoreGetCurrentAddress(&State), 0x2064, "Current address calculation");
    return 1;
}

void
RunCursorTests(void)
{
    printf("\n=== Cursor Engine Tests ===\n");

    TEST(TestCursorMovement, "Cursor Movement");
    TEST(TestCursorMovementByWidth, "Cursor Movement By Width");
    TEST(TestCursorBoundaries, "Cursor Boundaries");
    TEST(TestAlignToPage, "Align To Page");
    TEST(TestCurrentAddress, "Current Address");

    printf("\n=== Cursor Engine Tests Complete ===\n");
}
