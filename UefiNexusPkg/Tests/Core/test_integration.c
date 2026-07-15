/** @file
  Host-side integration tests for layered PageMem.

  This module validates the PageMem controller, action, and view flow by using
  mock adapters. These tests cover UI/Core interaction without requiring a real
  UEFI firmware environment.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <stdio.h>

#include "../../Adapter/AdapterInterface.h"
#include "../../UI/Pages/PageMem/PageMemLayered.h"
#include "test_common.h"

//
// Mock adapter accessors.
//
MEMORY_ADAPTER_INTERFACE *GetMockMemoryAdapter(void);
TUI_ADAPTER_INTERFACE    *GetMockTuiAdapter(void);
ERROR_ADAPTER_INTERFACE  *GetMockErrorAdapter(void);

//
// Mock TUI control helpers.
//
void MockTuiSetNextReadHex(NX_U64 Value, NX_BOOL ShouldSucceed);
void MockTuiReset(void);
const NX_UI_CHAR *MockTuiGetScreenBuffer(void);
NX_UINTN MockTuiGetLastReadHexMaxDigits(void);

//
// Mock error adapter control helpers.
//
NX_UINTN MockErrorGetCount(void);
void MockErrorReset(void);

//
// Mock memory adapter control helpers.
//
NX_UINTN MockMemoryGetDumpCallCount(void);
void MockMemoryReset(void);
NX_U64 MockMemoryGetValue(NX_U64 Address);

//
// Assertion helpers used by host-side integration tests.
//
#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return NX_FALSE; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return NX_FALSE; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

/**
  Build an adapter manager using mock adapters.

  @return Adapter manager populated with mock memory, TUI, and error adapters.
**/
static
ADAPTER_MANAGER
MakeMockAdapters(void)
{
    ADAPTER_MANAGER Adapters;

    Adapters.Memory = GetMockMemoryAdapter();
    Adapters.Tui    = GetMockTuiAdapter();
    Adapters.Error  = GetMockErrorAdapter();

    return Adapters;
}

/**
  Check whether the mock screen matches an expected string at a fixed position.

  @param[in] Screen    Mock screen buffer.
  @param[in] Row       Target row.
  @param[in] Column    Target column.
  @param[in] Expected  Expected string.

  @retval NX_TRUE   Expected string matches at the requested position.
  @retval NX_FALSE  Screen input is invalid or content does not match.
**/
static
NX_BOOL
ScreenMatchesAt(
    const NX_UI_CHAR *Screen,
    NX_UINTN          Row,
    NX_UINTN          Column,
    const NX_UI_CHAR *Expected
    )
{
    NX_UINTN Index;
    NX_UINTN Base;

    if (Screen == NULL || Expected == NULL) {
        return NX_FALSE;
    }

    Base = (Row * 80) + Column;

    for (Index = 0; Expected[Index] != L'\0'; Index++) {
        if (Screen[Base + Index] != Expected[Index]) {
            return NX_FALSE;
        }
    }

    return NX_TRUE;
}

/**
  Check whether the mock screen contains an expected string anywhere.

  @param[in] Screen    Mock screen buffer.
  @param[in] Expected  Expected string.

  @retval NX_TRUE   Expected string exists in the mock screen.
  @retval NX_FALSE  Screen input is invalid or string was not found.
**/
static
NX_BOOL
ScreenContains(
    const NX_UI_CHAR *Screen,
    const NX_UI_CHAR *Expected
    )
{
    NX_UINTN Start;
    NX_UINTN Index;

    if (Screen == NULL || Expected == NULL || Expected[0] == L'\0') {
        return NX_FALSE;
    }

    for (Start = 0; Start < (80 * 24); Start++) {
        for (Index = 0; Expected[Index] != L'\0'; Index++) {
            if (Screen[Start + Index] != Expected[Index]) {
                break;
            }
        }

        if (Expected[Index] == L'\0') {
            return NX_TRUE;
        }
    }

    return NX_FALSE;
}

//
// Validate PageMem controller initialization.
//
static
NX_BOOL
TestControllerInit(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;

    MockTuiReset();
    MockErrorReset();
    MockMemoryReset();

    ASSERT_EQ(PageMemControllerInit(&State, &Adapters), NX_UI_SUCCESS, "Controller init status");
    ASSERT_EQ(State.CursorState.Address, 0x1000, "Controller init address");
    ASSERT_TRUE(State.NeedsRedraw == NX_TRUE, "Controller init redraw");

    return NX_TRUE;
}

//
// Validate basic right-arrow cursor movement.
//
static
NX_BOOL
TestMoveKeyHandling(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { NX_UI_SCAN_RIGHT, 0 };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Handle right key");
    ASSERT_EQ(State.CursorState.Offset, 1, "Cursor moved right");

    return NX_TRUE;
}

//
// Validate edit flow writes the value returned by the mock TUI input.
//
static
NX_BOOL
TestEditFlow(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'e' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0xAA, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Edit key flow");
    ASSERT_EQ(MockMemoryGetValue(0x1000), 0xAA, "Memory write after edit");

    return NX_TRUE;
}

//
// Validate cursor movement respects selected access width.
//
static
NX_BOOL
TestWidthAwareCursorMove(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY SetWidthKey = { 0, L'4' };
    NX_UI_KEY MoveKey = { NX_UI_SCAN_RIGHT, 0 };

    PageMemControllerInit(&State, &Adapters);

    ASSERT_EQ(PageMemControllerHandleKeyPress(SetWidthKey, &State, &Adapters), NX_UI_SUCCESS, "Set width 4");
    ASSERT_EQ(PageMemControllerHandleKeyPress(MoveKey, &State, &Adapters), NX_UI_SUCCESS, "Move right at width 4");
    ASSERT_EQ(State.CursorState.Offset, 4, "Cursor moves by 4-byte cell");

    return NX_TRUE;
}

//
// Validate arrow-key movement uses partial redraw instead of full redraw.
//
static
NX_BOOL
TestArrowKeyUsesPartialUpdate(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY MoveKey = { NX_UI_SCAN_RIGHT, 0 };
    const NX_UI_CHAR *Screen;

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    State.NeedsRedraw = NX_FALSE;

    ASSERT_EQ(PageMemControllerHandleKeyPress(MoveKey, &State, &Adapters), NX_UI_SUCCESS, "Arrow key partial update status");
    ASSERT_TRUE(State.NeedsRedraw == NX_FALSE, "Arrow key does not request full redraw");

    Screen = MockTuiGetScreenBuffer();
    ASSERT_EQ(State.CursorState.Address, 0x1000, "Arrow key keeps page base address");
    ASSERT_TRUE(ScreenContains(Screen, L"Offset: 01"), "Arrow key updates info row");

    return NX_TRUE;
}

//
// Validate edit flow passes width-based max digit count to TUI input.
//
static
NX_BOOL
TestEditFlowUsesWidthDigits(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY WidthKey = { 0, L'2' };
    NX_UI_KEY EditKey = { 0, L'e' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();

    ASSERT_EQ(PageMemControllerHandleKeyPress(WidthKey, &State, &Adapters), NX_UI_SUCCESS, "Set width 2 for edit");

    MockTuiSetNextReadHex(0xABCD, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(EditKey, &State, &Adapters), NX_UI_SUCCESS, "Edit key width 2");
    ASSERT_EQ(MockTuiGetLastReadHexMaxDigits(), 4, "Edit uses width-based max digits");

    return NX_TRUE;
}

//
// Validate edit flow rejects values that exceed selected access width.
//
static
NX_BOOL
TestEditFlowRejectsOverflow(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY WidthKey = { 0, L'2' };
    NX_UI_KEY EditKey = { 0, L'e' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    MockErrorReset();

    ASSERT_EQ(PageMemControllerHandleKeyPress(WidthKey, &State, &Adapters), NX_UI_SUCCESS, "Set width 2 for overflow test");

    MockTuiSetNextReadHex(0x12345, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(EditKey, &State, &Adapters), NX_UI_INVALID_PARAMETER, "Reject overflow edit");
    ASSERT_TRUE(MockErrorGetCount() > 0, "Overflow edit reports error");
    ASSERT_EQ(MockMemoryGetValue(0x1000), 0x0102030405060708ULL, "Overflow edit does not write memory");

    return NX_TRUE;
}

//
// Validate goto flow aligns target address and stores offset.
//
static
NX_BOOL
TestGotoFlow(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'g' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0x1080, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Goto key flow");
    ASSERT_EQ(State.CursorState.Address, 0x1000, "Goto aligned address");
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Goto offset");

    return NX_TRUE;
}

//
// Validate invalid goto reports error.
//
static
NX_BOOL
TestInvalidGotoReportsError(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'g' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0x9000, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_INVALID_PARAMETER, "Invalid goto status");
    ASSERT_TRUE(MockErrorGetCount() > 0, "Invalid goto reports error");

    return NX_TRUE;
}

//
// Validate ranges command invokes memory adapter dump flow.
//
static
NX_BOOL
TestRangesCommand(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'r' };

    PageMemControllerInit(&State, &Adapters);

    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Ranges command status");
    ASSERT_TRUE(MockMemoryGetDumpCallCount() > 0, "Ranges command triggers dump");

    return NX_TRUE;
}

//
// Validate base page drawing.
//
static
NX_BOOL
TestViewDraw(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const NX_UI_CHAR *Screen;

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();

    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();

    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_TITLE_ROW, 0, L"Memory Viewer"), "View draws page header");

    return NX_TRUE;
}

//
// Validate view rendering with 2-byte access width.
//
static
NX_BOOL
TestViewDrawWidth2(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const NX_UI_CHAR *Screen;
    NX_UI_KEY Key = { 0, L'2' };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Set width 2");

    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();

    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_HEADER_ROW, 4, L"0100 "), "Width 2 header");
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 4, L"0708 "), "Width 2 cell");

    return NX_TRUE;
}

//
// Validate view rendering with 4-byte access width.
//
static
NX_BOOL
TestViewDrawWidth4(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const NX_UI_CHAR *Screen;
    NX_UI_KEY Key = { 0, L'4' };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Set width 4");

    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();

    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_HEADER_ROW, 4, L"03020100 "), "Width 4 header");
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 4, L"05060708 "), "Width 4 cell");

    return NX_TRUE;
}

/**
  Run all PageMem layered integration tests.
**/
void
RunIntegrationTests(void)
{
    printf("\n=== Layered Integration Tests ===\n");

    TEST(TestControllerInit, "Controller Init");
    TEST(TestMoveKeyHandling, "Move Key Handling");
    TEST(TestWidthAwareCursorMove, "Width Aware Cursor Move");
    TEST(TestArrowKeyUsesPartialUpdate, "Arrow Key Uses Partial Update");
    TEST(TestEditFlow, "Edit Flow");
    TEST(TestEditFlowUsesWidthDigits, "Edit Flow Uses Width Digits");
    TEST(TestEditFlowRejectsOverflow, "Edit Flow Rejects Overflow");
    TEST(TestGotoFlow, "Goto Flow");
    TEST(TestInvalidGotoReportsError, "Invalid Goto Error");
    TEST(TestRangesCommand, "Ranges Command");
    TEST(TestViewDraw, "View Draw");
    TEST(TestViewDrawWidth2, "View Draw Width 2");
    TEST(TestViewDrawWidth4, "View Draw Width 4");

    printf("\n=== Layered Integration Tests Complete ===\n");
}