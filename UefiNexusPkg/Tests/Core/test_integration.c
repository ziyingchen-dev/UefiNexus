/**
 * test_integration.c - Host-side integration tests for layered PageMem
 */

#include <stdio.h>

#include "../../Adapter/AdapterInterface.h"
#include "../../UI/Pages/PageMem/PageMemLayered.h"
#include "test_common.h"

MEMORY_ADAPTER_INTERFACE* GetMockMemoryAdapter(void);
TUI_ADAPTER_INTERFACE* GetMockTuiAdapter(void);
ERROR_ADAPTER_INTERFACE* GetMockErrorAdapter(void);
void MockTuiSetNextReadHex(PM_U64 Value, PM_BOOL ShouldSucceed);
void MockTuiReset(void);
const PM_UI_CHAR* MockTuiGetScreenBuffer(void);
PM_UINTN MockTuiGetLastReadHexMaxDigits(void);
PM_UINTN MockErrorGetCount(void);
void MockErrorReset(void);
PM_UINTN MockMemoryGetDumpCallCount(void);
void MockMemoryReset(void);
PM_U64 MockMemoryGetValue(PM_U64 Address);

#define ASSERT_EQ(actual, expected, test_name) \
    do { \
        if ((actual) != (expected)) { \
            printf("[FAIL] %s: expected %llu, got %llu\n", test_name, (unsigned long long)(expected), (unsigned long long)(actual)); \
            return PM_FALSE; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

#define ASSERT_TRUE(cond, test_name) \
    do { \
        if (!(cond)) { \
            printf("[FAIL] %s: condition is false\n", test_name); \
            return PM_FALSE; \
        } else { \
            printf("[PASS] %s\n", test_name); \
        } \
    } while (0)

static ADAPTER_MANAGER
MakeMockAdapters(void)
{
    ADAPTER_MANAGER Adapters;

    Adapters.Memory = GetMockMemoryAdapter();
    Adapters.Tui = GetMockTuiAdapter();
    Adapters.Error = GetMockErrorAdapter();
    return Adapters;
}

static PM_BOOL
ScreenMatchesAt(
    const PM_UI_CHAR *Screen,
    PM_UINTN Row,
    PM_UINTN Column,
    const PM_UI_CHAR *Expected
    )
{
    PM_UINTN Index;
    PM_UINTN Base;

    if (Screen == NULL || Expected == NULL) {
        return PM_FALSE;
    }

    Base = (Row * 80) + Column;
    for (Index = 0; Expected[Index] != L'\0'; Index++) {
        if (Screen[Base + Index] != Expected[Index]) {
            return PM_FALSE;
        }
    }

    return PM_TRUE;
}

static PM_BOOL
ScreenContains(
    const PM_UI_CHAR *Screen,
    const PM_UI_CHAR *Expected
    )
{
    PM_UINTN Start;
    PM_UINTN Index;

    if (Screen == NULL || Expected == NULL || Expected[0] == L'\0') {
        return PM_FALSE;
    }

    for (Start = 0; Start < (80 * 24); Start++) {
        for (Index = 0; Expected[Index] != L'\0'; Index++) {
            if (Screen[Start + Index] != Expected[Index]) {
                break;
            }
        }

        if (Expected[Index] == L'\0') {
            return PM_TRUE;
        }
    }

    return PM_FALSE;
}

static PM_BOOL
TestControllerInit(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;

    MockTuiReset();
    MockErrorReset();
    MockMemoryReset();

    ASSERT_EQ(PageMemControllerInit(&State, &Adapters), PM_UI_SUCCESS, "Controller init status");
    ASSERT_EQ(State.CursorState.Address, 0x1000, "Controller init address");
    ASSERT_TRUE(State.NeedsRedraw == PM_TRUE, "Controller init redraw");

    return PM_TRUE;
}

static PM_BOOL
TestMoveKeyHandling(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY Key = { PM_UI_SCAN_RIGHT, 0 };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_SUCCESS, "Handle right key");
    ASSERT_EQ(State.CursorState.Offset, 1, "Cursor moved right");

    return PM_TRUE;
}

static PM_BOOL
TestEditFlow(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY Key = { 0, L'e' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0xAA, PM_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_SUCCESS, "Edit key flow");
    ASSERT_EQ(MockMemoryGetValue(0x1000), 0xAA, "Memory write after edit");

    return PM_TRUE;
}

static PM_BOOL
TestWidthAwareCursorMove(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY SetWidthKey = { 0, L'4' };
    PM_UI_KEY MoveKey = { PM_UI_SCAN_RIGHT, 0 };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(SetWidthKey, &State, &Adapters), PM_UI_SUCCESS, "Set width 4");
    ASSERT_EQ(PageMemControllerHandleKeyPress(MoveKey, &State, &Adapters), PM_UI_SUCCESS, "Move right at width 4");
    ASSERT_EQ(State.CursorState.Offset, 4, "Cursor moves by 4-byte cell");

    return PM_TRUE;
}

static PM_BOOL
TestArrowKeyUsesPartialUpdate(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY MoveKey = { PM_UI_SCAN_RIGHT, 0 };
    const PM_UI_CHAR *Screen;

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    State.NeedsRedraw = PM_FALSE;

    ASSERT_EQ(PageMemControllerHandleKeyPress(MoveKey, &State, &Adapters), PM_UI_SUCCESS, "Arrow key partial update status");
    ASSERT_TRUE(State.NeedsRedraw == PM_FALSE, "Arrow key does not request full redraw");

    Screen = MockTuiGetScreenBuffer();
    ASSERT_TRUE(ScreenContains(Screen, L"1000"), "Arrow key keeps page base address");
    ASSERT_TRUE(ScreenContains(Screen, L"Offset: 01"), "Arrow key updates info row");

    return PM_TRUE;
}

static PM_BOOL
TestEditFlowUsesWidthDigits(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY WidthKey = { 0, L'2' };
    PM_UI_KEY EditKey = { 0, L'e' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    ASSERT_EQ(PageMemControllerHandleKeyPress(WidthKey, &State, &Adapters), PM_UI_SUCCESS, "Set width 2 for edit");
    MockTuiSetNextReadHex(0xABCD, PM_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(EditKey, &State, &Adapters), PM_UI_SUCCESS, "Edit key width 2");
    ASSERT_EQ(MockTuiGetLastReadHexMaxDigits(), 4, "Edit uses width-based max digits");

    return PM_TRUE;
}

static PM_BOOL
TestEditFlowRejectsOverflow(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY WidthKey = { 0, L'2' };
    PM_UI_KEY EditKey = { 0, L'e' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    MockErrorReset();
    ASSERT_EQ(PageMemControllerHandleKeyPress(WidthKey, &State, &Adapters), PM_UI_SUCCESS, "Set width 2 for overflow test");
    MockTuiSetNextReadHex(0x12345, PM_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(EditKey, &State, &Adapters), PM_UI_INVALID_PARAMETER, "Reject overflow edit");
    ASSERT_TRUE(MockErrorGetCount() > 0, "Overflow edit reports error");
    ASSERT_EQ(MockMemoryGetValue(0x1000), 0x0102030405060708ULL, "Overflow edit does not write memory");

    return PM_TRUE;
}

static PM_BOOL
TestGotoFlow(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY Key = { 0, L'g' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0x1080, PM_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_SUCCESS, "Goto key flow");
    ASSERT_EQ(State.CursorState.Address, 0x1000, "Goto aligned address");
    ASSERT_EQ(State.CursorState.Offset, 0x80, "Goto offset");

    return PM_TRUE;
}

static PM_BOOL
TestInvalidGotoReportsError(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY Key = { 0, L'g' };

    PageMemControllerInit(&State, &Adapters);
    MockTuiSetNextReadHex(0x9000, PM_TRUE);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_INVALID_PARAMETER, "Invalid goto status");
    ASSERT_TRUE(MockErrorGetCount() > 0, "Invalid goto reports error");

    return PM_TRUE;
}

static PM_BOOL
TestRangesCommand(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    PM_UI_KEY Key = { 0, L'r' };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_SUCCESS, "Ranges command status");
    ASSERT_TRUE(MockMemoryGetDumpCallCount() > 0, "Ranges command triggers dump");

    return PM_TRUE;
}

static PM_BOOL
TestViewDraw(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const PM_UI_CHAR *Screen;

    PageMemControllerInit(&State, &Adapters);
    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_TITLE_ROW, 0, L"Memory Viewer"), "View draws page header");

    return PM_TRUE;
}

static PM_BOOL
TestViewDrawWidth2(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const PM_UI_CHAR *Screen;
    PM_UI_KEY Key = { 0, L'2' };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_SUCCESS, "Set width 2");

    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();

    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_HEADER_ROW, ADDR_PREFIX_LEN, L"0100 "), "Width 2 header");
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, ADDR_PREFIX_LEN, L"0708 "), "Width 2 cell");

    return PM_TRUE;
}

static PM_BOOL
TestViewDrawWidth4(void)
{
    ADAPTER_MANAGER Adapters = MakeMockAdapters();
    PAGEMEM_PAGE_STATE State;
    const PM_UI_CHAR *Screen;
    PM_UI_KEY Key = { 0, L'4' };

    PageMemControllerInit(&State, &Adapters);
    ASSERT_EQ(PageMemControllerHandleKeyPress(Key, &State, &Adapters), PM_UI_SUCCESS, "Set width 4");

    MockTuiReset();
    PageMemViewDrawPage(Adapters.Tui, Adapters.Memory, &State);
    Screen = MockTuiGetScreenBuffer();

    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_HEADER_ROW, ADDR_PREFIX_LEN, L"03020100 "), "Width 4 header");
    ASSERT_TRUE(ScreenMatchesAt(Screen, PAGEMEM_DATA_ROW, ADDR_PREFIX_LEN, L"05060708 "), "Width 4 cell");

    return PM_TRUE;
}

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
