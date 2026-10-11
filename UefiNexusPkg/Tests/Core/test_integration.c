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
void MockTuiSetNextReadHexEcho(const NX_UI_CHAR *Echo);
void MockTuiReset(void);
const NX_UI_CHAR *MockTuiGetScreenBuffer(void);
NX_UINTN MockTuiGetLastReadHexMaxDigits(void);
void MockTuiGetReadHexCursorPosition(NX_UINTN *Column, NX_UINTN *Row);
const NX_UI_CHAR *MockTuiGetReadHexScreenRow(void);
const NX_UI_CHAR *MockTuiGetReadHexEchoedScreenRow(void);

//
// Mock error adapter control helpers.
//
NX_UINTN MockErrorGetCount(void);
void MockErrorReset(void);

//
// Mock memory adapter control helpers.
//
NX_UINTN MockMemoryGetDumpCallCount(void);
NX_UINTN MockMemoryGetTypeLookupCount(void);
NX_UINTN MockMemoryGetPageAccessQueryCount(void);
NX_U64 MockMemoryGetLastAccessBase(void);
NX_U64 MockMemoryGetLastAccessLength(void);
NX_U64 MockMemoryGetLastPageAccessBase(void);
NX_U64 MockMemoryGetLastPageAccessLength(void);
void MockMemorySetAccessForDescriptor(NX_UINTN Index, NX_BOOL ReadAllowed, NX_BOOL WriteAllowed, NX_BOOL AccessKnown);
void MockMemoryReset(void);
NX_U64 MockMemoryGetValue(NX_U64 Address);
void MockMemorySetReadFailure(NX_U64 Address);

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

static
NX_BOOL
ScreenSuffixIsBlank(
    const NX_UI_CHAR *Screen,
    NX_UINTN          Row,
    NX_UINTN          Column
    )
{
    for (; Column < 80; Column++) {
        if (Screen[Row * 80 + Column] != L' ') {
            return NX_FALSE;
        }
    }

    return NX_TRUE;
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

    MockTuiReset();
    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Handle right key");
    ASSERT_EQ(State.CursorState.Offset, 1, "Cursor moved right");
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001001  Offset: 01  Width: 1"),
        "Bottom address status updates after cursor movement"
        );

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

    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(
        ScreenMatchesAt(
            MockTuiGetScreenBuffer(),
            PAGEMEM_TYPE_ROW,
            0,
            L"Type: EfiConventionalMemory  Access: R/W"
            ),
        "Same-page Goto keeps page type title"
        );
    ASSERT_TRUE(!ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_TYPE_ROW, 0, L"[DATA]"),
                "Viewer title no longer includes Tag");

    return NX_TRUE;
}

//
// Validate Goto prompt placement, input echo, and cleanup for all outcomes.
//
static
NX_BOOL
TestGotoPromptLifecycle(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'g' };
    NX_UINTN PromptColumn;
    NX_UINTN PromptRow;

    MockTuiReset();
    PageMemControllerInit(&State, &Adapters);
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW, 0, PAGEMEM_HELP_TEXT),
        "Goto baseline help is initially displayed"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001000  Offset: 00  Width: 1"),
        "Goto starts without replacing address status"
        );

    MockTuiSetNextReadHex(0x1080, NX_TRUE);
    MockTuiSetNextReadHexEcho(L"1080");
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS,
              "Goto succeeds");
    MockTuiGetReadHexCursorPosition(&PromptColumn, &PromptRow);
    ASSERT_EQ(PromptColumn, PAGEMEM_GOTO_PROMPT_COLUMN +
                            (sizeof(L"Goto address (hex):") / sizeof(NX_UI_CHAR)) - 1,
              "Goto input cursor follows the colon");
    ASSERT_EQ(PromptRow, PAGEMEM_HELP_ROW, "Goto prompt and input use help row");
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetReadHexScreenRow(), 0, 0,
                        L"Arrows: Move  N/P: Page  Goto address (hex):"),
        "Goto prompt follows baseline help with two spaces"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetReadHexEchoedScreenRow(), 0, PromptColumn, L"1080"),
        "Goto input is echoed directly after prompt"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW, 0, PAGEMEM_HELP_TEXT),
        "Successful Goto restores baseline help"
        );
    ASSERT_TRUE(
        ScreenSuffixIsBlank(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW,
                            PAGEMEM_GOTO_PROMPT_COLUMN),
        "Successful Goto clears only the prompt suffix"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001000  Offset: 00  Width: 1"),
        "Successful Goto prompt does not clear address row"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP2_ROW, 0,
                        L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit"),
        "Successful Goto prompt preserves bottom help"
        );
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Successful Goto updates cursor state");

    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001080  Offset: 80  Width: 1"),
        "Successful Goto updates address status"
        );

    MockTuiSetNextReadHex(0x10A0, NX_FALSE);
    ASSERT_EQ(
        PageMemControllerHandleKeyPress(Key, &State, &Adapters),
        NX_UI_ABORTED,
        "Cancelled Goto is handled"
        );
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Cancelled Goto preserves cursor state");
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW, 0, PAGEMEM_HELP_TEXT) &&
        ScreenSuffixIsBlank(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW,
                            PAGEMEM_GOTO_PROMPT_COLUMN),
        "Cancelled Goto restores help and clears prompt suffix"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001080  Offset: 80  Width: 1"),
        "Cancelled Goto preserves address status"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP2_ROW, 0,
                        L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit"),
        "Cancelled Goto preserves bottom help"
        );

    MockTuiSetNextReadHex(0x10C0, NX_FALSE);
    ASSERT_EQ(
        PageMemControllerHandleKeyPress(Key, &State, &Adapters),
        NX_UI_ABORTED,
        "Failed Goto input is handled"
        );
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Failed Goto preserves cursor state");
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW, 0, PAGEMEM_HELP_TEXT) &&
        ScreenSuffixIsBlank(MockTuiGetScreenBuffer(), PAGEMEM_HELP_ROW,
                            PAGEMEM_GOTO_PROMPT_COLUMN),
        "Failed Goto restores help and clears prompt suffix"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001080  Offset: 80  Width: 1"),
        "Failed Goto preserves address status"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_HELP2_ROW, 0,
                        L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit"),
        "Failed Goto preserves bottom help"
        );

    return NX_TRUE;
}

//
// Validate goto to an unmapped hole changes page and renders an unknown cell.
//
static
NX_BOOL
TestGotoHoleNavigatesAndShowsUnknown(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'g' };
    const NX_UI_CHAR *Screen;

    MockTuiReset();
    PageMemControllerInit(&State, &Adapters);
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    MockErrorReset();
    MockTuiSetNextReadHex(0x9080, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Goto hole status");
    ASSERT_EQ(State.CursorState.Address, 0x9000, "Goto hole page address");
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Goto hole offset");
    ASSERT_EQ(MockErrorGetCount(), 0, "Goto hole does not report error");
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001000  Offset: 00  Width: 1"),
        "Goto prompt preserves the previous address row until redraw"
        );

    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000009080  Offset: 80  Width: 1"),
        "Goto redraw updates address status"
        );
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 4, L"??"), "Hole cell displays unknown marker");
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0, L"Type: Unmapped  Access: --"),
        "Hole page title is distinct"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0,
                        L"Type: Unmapped  Access: --"),
        "Hole page access is denied by tool policy"
        );
    ASSERT_TRUE(!ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0, L"[HOLE]"),
                "Hole title does not include Tag");

    return NX_TRUE;
}

// Validate goto to an unsupported descriptor is allowed and renders ??.
//
static
NX_BOOL
TestGotoUnsupportedNavigatesAndShowsUnknown(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY Key = { 0, L'g' };
    const NX_UI_CHAR *Screen;

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0x2080, NX_TRUE);

    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), NX_UI_SUCCESS, "Goto unsupported status");
    ASSERT_EQ(State.CursorState.Address, 0x2000, "Goto unsupported page address");
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Goto unsupported offset");

    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 4, L"??"), "Unsupported cell displays unknown marker");
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0, L"Type: Unknown(14)  Access: --"),
        "Unknown descriptor title retains numeric type"
        );
    ASSERT_TRUE(!ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0, L"[UNK]"),
                "Unknown type title does not include Tag");

    return NX_TRUE;
}

// Validate that a failed read is rendered as unknown, not as a zero value.
//
static
NX_BOOL
TestReadFailureDisplaysUnknown(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const NX_UI_CHAR *Screen;

    PageMemControllerInit(&State, &Adapters);
    MockMemorySetReadFailure(0x1000);
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();

    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 4, L"??"), "Failed read displays unknown marker");
    ASSERT_TRUE(!ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 4, L"00"), "Failed read does not display zero");
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, 7, L"08"),
                "Successful neighboring read still displays data");
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0,
                                L"Type: EfiConventionalMemory  Access: R/W"),
                "Valid RAM page access is R/W");

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

    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_TITLE_ROW, 0, L"Memory Viewer "),
        "First row contains only page title"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_TYPE_ROW, 0,
                        L"Type: EfiConventionalMemory  Access: R/W"),
        "Second row contains type and access"
        );
    ASSERT_TRUE(
        !ScreenMatchesAt(Screen, PAGEMEM_TITLE_ROW, 0, L"Type:"),
        "First row has no extra fields"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_INFO_ROW, 0, L"Addr: 0x0000000000001000  Offset: 00  Width: 1"),
        "Address status is below the memory grid"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_HEADER_ROW, PAGEMEM_DATA_COLUMN, L"00"),
        "Memory grid header remains on its original row"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW + PAGEMEM_DATA_ROWS - 1, 0, L"F0:"),
        "Memory grid last row remains on its original row"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_HELP_ROW, 0, L"Arrows: Move  N/P: Page"),
        "First operation prompt follows address status"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(Screen, PAGEMEM_HELP2_ROW, 0,
                        L"E: Edit  G: Goto  R: Ranges  1/2/4/8: Width  ESC: Exit"),
        "Second operation prompt remains at the bottom"
        );

    return NX_TRUE;
}

//
// Validate page changes update the title using the page-base descriptor.
//
static
NX_BOOL
TestPageNavigationUpdatesTitle(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY PageDownKey = { 0, L'n' };
    NX_UI_KEY GotoKey = { 0, L'g' };

    MockMemoryReset();
    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 1, "Initialization looks up page type once");

    for (NX_UINTN Index = 0; Index < 16; Index++) {
        ASSERT_EQ(
            PageMemControllerHandleKeyPress(PageDownKey, &State, &Adapters),
            NX_UI_SUCCESS,
            "Page down status"
            );
    }

    ASSERT_EQ(State.CursorState.Address, 0x3000, "Page down reaches ACPI descriptor");
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 17, "Each changed page refreshes page type once");
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(
        ScreenMatchesAt(
            MockTuiGetScreenBuffer(),
            PAGEMEM_TYPE_ROW,
            0,
            L"Type: EfiACPIReclaimMemory  Access: R/-"
            ),
        "Page navigation updates memory type title"
        );

    MockTuiSetNextReadHex(0x2080, NX_TRUE);
    ASSERT_EQ(
        PageMemControllerHandleKeyPress(GotoKey, &State, &Adapters),
        NX_UI_SUCCESS,
        "Goto unknown descriptor status"
        );
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 18, "Goto refreshes page type once");
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(
        ScreenMatchesAt(
            MockTuiGetScreenBuffer(),
            PAGEMEM_TYPE_ROW,
            0,
            L"Type: Unknown(14)  Access: --"
            ),
        "Goto updates memory type title"
        );

    return NX_TRUE;
}

//
// Verify same-page cursor and width changes keep cached page metadata.
//
static
NX_BOOL
TestSamePageChangesDoNotLookupDescriptor(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY MoveKey = { NX_UI_SCAN_RIGHT, 0 };
    NX_UI_KEY WidthKey = { 0, L'4' };

    MockMemoryReset();
    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 1, "Initial page type lookup count");
    ASSERT_EQ(MockMemoryGetPageAccessQueryCount(), 1, "Initial page access lookup count");
    ASSERT_EQ(MockMemoryGetLastPageAccessBase(), 0x1000, "Page access uses current page base");
    ASSERT_EQ(MockMemoryGetLastPageAccessLength(), 256, "Page access queries full page");

    ASSERT_EQ(
        PageMemControllerHandleKeyPress(MoveKey, &State, &Adapters),
        NX_UI_SUCCESS,
        "Same-page cursor move status"
        );
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 1, "Cursor move does not look up descriptor");
    ASSERT_EQ(MockMemoryGetPageAccessQueryCount(), 1, "Cursor move reuses page access");
    ASSERT_TRUE(
        ScreenMatchesAt(
            MockTuiGetScreenBuffer(),
            PAGEMEM_TYPE_ROW,
            0,
            L"Type: EfiConventionalMemory  Access: R/W"
            ),
        "Cursor move preserves page title"
        );

    ASSERT_EQ(
        PageMemControllerHandleKeyPress(WidthKey, &State, &Adapters),
        NX_UI_SUCCESS,
        "Same-page width change status"
        );
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 1, "Width change does not look up descriptor");
    ASSERT_EQ(MockMemoryGetPageAccessQueryCount(), 1, "Width change reuses page access");
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_EQ(MockMemoryGetTypeLookupCount(), 1, "Redraw uses cached page type");
    ASSERT_TRUE(
        ScreenMatchesAt(
            MockTuiGetScreenBuffer(),
            PAGEMEM_TYPE_ROW,
            0,
            L"Type: EfiConventionalMemory  Access: R/W"
            ),
        "Width change preserves page title"
        );
    ASSERT_TRUE(
        ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_INFO_ROW, 0,
                        L"Addr: 0x0000000000001000  Offset: 00  Width: 4"),
        "Bottom address row reflects updated width"
        );

    return NX_TRUE;
}

static
NX_BOOL
TestPageAccessTitleTracksProtection(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    NX_UI_KEY GotoKey = { 0, L'g' };

    MockMemoryReset();
    ASSERT_EQ(PageMemControllerInit(&State, &Adapters), NX_UI_SUCCESS, "Access title controller init");

    MockMemorySetAccessForDescriptor(2, NX_TRUE, NX_FALSE, NX_TRUE);
    MockTuiSetNextReadHex(0x3080, NX_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(GotoKey, &State, &Adapters),
              NX_UI_SUCCESS,
              "Navigate to read-only page");
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_TYPE_ROW, 0,
                                L"Type: EfiACPIReclaimMemory  Access: R/-"),
                "Read-only page title shows R slash minus");

    MockMemorySetAccessForDescriptor(0, NX_FALSE, NX_TRUE, NX_TRUE);
    MockTuiSetNextReadHex(0x1080, NX_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(GotoKey, &State, &Adapters),
              NX_UI_SUCCESS,
              "Goto read-protected page");
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_TYPE_ROW, 0,
                                L"Type: EfiConventionalMemory  Access: -/W"),
                "Read-protected page title shows minus slash W");

    MockMemorySetAccessForDescriptor(0, NX_FALSE, NX_FALSE, NX_TRUE);
    MockTuiSetNextReadHex(0x2080, NX_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(GotoKey, &State, &Adapters),
              NX_UI_SUCCESS,
              "Leave page before refreshing no-access title");
    MockTuiSetNextReadHex(0x1080, NX_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(GotoKey, &State, &Adapters),
              NX_UI_SUCCESS,
              "Goto page with no read/write access");
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    ASSERT_TRUE(ScreenMatchesAt(MockTuiGetScreenBuffer(), PAGEMEM_TYPE_ROW, 0,
                                L"Type: EfiConventionalMemory  Access: --"),
                "No read/write access title uses double dash");

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
    TEST(TestGotoPromptLifecycle, "Goto Prompt Lifecycle");
    TEST(TestGotoHoleNavigatesAndShowsUnknown, "Goto Hole");
    TEST(TestGotoUnsupportedNavigatesAndShowsUnknown, "Goto Unsupported Type");
    TEST(TestPageNavigationUpdatesTitle, "Page Navigation Updates Type Title");
    TEST(TestSamePageChangesDoNotLookupDescriptor, "Same-Page Changes Reuse Type");
    TEST(TestPageAccessTitleTracksProtection, "Page Access Title Tracks Protection");
    TEST(TestReadFailureDisplaysUnknown, "Read Failure Placeholder");
    TEST(TestRangesCommand, "Ranges Command");
    TEST(TestViewDraw, "View Draw");
    TEST(TestViewDrawWidth2, "View Draw Width 2");
    TEST(TestViewDrawWidth4, "View Draw Width 4");

    printf("\n=== Layered Integration Tests Complete ===\n");
}