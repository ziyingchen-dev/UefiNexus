/** @file
  Mock TUI adapter for host-side tests.

  This module implements TUI_ADAPTER_INTERFACE for PageMem host-side integration
  tests. It provides an in-memory text screen, cursor state, key input queue,
  and controlled hexadecimal input behavior so UI rendering and controller
  actions can be verified without a real UEFI console.

  Copyright (c) 2026, ziyingchen-dev
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "../../Adapter/AdapterInterface.h"

//
// Mock screen state.
//
static NX_UI_CHAR gMockScreenBuffer[80 * 24];
static NX_UINTN   gMockCursorColumn = 0;
static NX_UINTN   gMockCursorRow    = 0;
static NX_UINTN   gMockLastAttribute = 0;
static NX_UINTN   gMockScreenWidth  = 80;
static NX_UINTN   gMockScreenHeight = 24;

//
// Mock key input queue.
//
static NX_UI_KEY gMockKeyQueue[32];
static NX_UINTN  gMockKeyHead = 0;
static NX_UINTN  gMockKeyTail = 0;

//
// Mock ReadHex() state.
//
static NX_U64   gMockNextHexValue = 0;
static NX_BOOL  gMockReadHexShouldSucceed = NX_TRUE;
static NX_UINTN gMockLastReadHexMaxDigits = 0;

static void MockTuiClearScreen(void);
static void MockTuiSetAttribute(NX_UINTN Attribute);
static void MockTuiSetCursorPosition(NX_UINTN Column, NX_UINTN Row);
static void MockTuiOutputString(const NX_UI_CHAR *String);
static void MockTuiOutputStringAt(NX_UINTN Column, NX_UINTN Row, const NX_UI_CHAR *String);
static void MockTuiDrawHeader(const NX_UI_CHAR *Title);
static void MockTuiDrawFooter(const NX_UI_CHAR *Help);
static NX_UI_KEY MockTuiReadKey(void);
static NX_BOOL MockTuiReadHex(NX_U64 *Value, NX_UINTN MaxDigits);
static NX_UI_STATUS MockTuiGetScreenDimensions(NX_UINTN *Columns, NX_UINTN *Rows);

//
// Mock TUI adapter instance used by host-side integration tests.
//
static TUI_ADAPTER_INTERFACE mTuiAdapterMock = {
    .ClearScreen         = MockTuiClearScreen,
    .SetAttribute        = MockTuiSetAttribute,
    .SetCursorPosition   = MockTuiSetCursorPosition,
    .OutputString        = MockTuiOutputString,
    .OutputStringAt      = MockTuiOutputStringAt,
    .DrawHeader          = MockTuiDrawHeader,
    .DrawFooter          = MockTuiDrawFooter,
    .ReadKey             = MockTuiReadKey,
    .ReadHex             = MockTuiReadHex,
    .GetScreenDimensions = MockTuiGetScreenDimensions,
};

static
void
MockTuiClearScreen(void)
{
    for (NX_UINTN Index = 0; Index < sizeof(gMockScreenBuffer) / sizeof(NX_UI_CHAR); Index++) {
        gMockScreenBuffer[Index] = L' ';
    }

    gMockCursorColumn = 0;
    gMockCursorRow    = 0;
}

static
void
MockTuiSetAttribute(
    NX_UINTN Attribute
    )
{
    gMockLastAttribute = Attribute;
}

static
void
MockTuiSetCursorPosition(
    NX_UINTN Column,
    NX_UINTN Row
    )
{
    gMockCursorColumn = Column;
    gMockCursorRow    = Row;
}

static
void
MockTuiOutputString(
    const NX_UI_CHAR *String
    )
{
    if (String == NULL) {
        return;
    }

    while (*String != L'\0' && gMockCursorColumn < gMockScreenWidth) {
        NX_UINTN Index;

        Index = gMockCursorRow * gMockScreenWidth + gMockCursorColumn;

        gMockScreenBuffer[Index] = *String;
        gMockCursorColumn++;
        String++;
    }
}

static
void
MockTuiOutputStringAt(
    NX_UINTN          Column,
    NX_UINTN          Row,
    const NX_UI_CHAR *String
    )
{
    MockTuiSetCursorPosition(Column, Row);
    MockTuiOutputString(String);
}

static
void
MockTuiDrawHeader(
    const NX_UI_CHAR *Title
    )
{
    MockTuiOutputStringAt(0, 0, Title);
}

static
void
MockTuiDrawFooter(
    const NX_UI_CHAR *Help
    )
{
    MockTuiOutputStringAt(0, gMockScreenHeight - 1, Help);
}

static
NX_UI_KEY
MockTuiReadKey(void)
{
    NX_UI_KEY Key = { 0 };

    if (gMockKeyHead < gMockKeyTail) {
        Key = gMockKeyQueue[gMockKeyHead++];
    }

    return Key;
}

static
NX_BOOL
MockTuiReadHex(
    NX_U64   *Value,
    NX_UINTN MaxDigits
    )
{
    gMockLastReadHexMaxDigits = MaxDigits;

    if (Value != NULL) {
        *Value = gMockNextHexValue;
    }

    return gMockReadHexShouldSucceed;
}

static
NX_UI_STATUS
MockTuiGetScreenDimensions(
    NX_UINTN *Columns,
    NX_UINTN *Rows
    )
{
    if (Columns != NULL) {
        *Columns = gMockScreenWidth;
    }

    if (Rows != NULL) {
        *Rows = gMockScreenHeight;
    }

    return NX_UI_SUCCESS;
}

/**
  Retrieve the mock TUI adapter.

  @retval Other  Pointer to the mock TUI_ADAPTER_INTERFACE instance.
**/
TUI_ADAPTER_INTERFACE *
GetMockTuiAdapter(void)
{
    return &mTuiAdapterMock;
}

/*
  Append captured text output to the host-side mock TUI buffer.
  This is the symbol expected by HostStubs.c and test harnesses.
*/
void
MockTuiAppendString(
    const NX_UI_CHAR *String
    )
{
    MockTuiOutputString(String);
}

/**
  Retrieve the mock screen buffer for verification.

  @return Pointer to the mock screen buffer.
**/
const NX_UI_CHAR *
MockTuiGetScreenBuffer(void)
{
    return gMockScreenBuffer;
}

/**
  Retrieve the current mock cursor position.

  @param[out] Column  Optional pointer that receives cursor column.
  @param[out] Row     Optional pointer that receives cursor row.
**/
void
MockTuiGetCursorPosition(
    NX_UINTN *Column,
    NX_UINTN *Row
    )
{
    if (Column != NULL) {
        *Column = gMockCursorColumn;
    }

    if (Row != NULL) {
        *Row = gMockCursorRow;
    }
}

/**
  Push one key into the mock input queue.

  @param[in] Key  Key to enqueue.
**/
void
MockTuiPushKey(
    NX_UI_KEY Key
    )
{
    if (gMockKeyTail < (sizeof(gMockKeyQueue) / sizeof(gMockKeyQueue[0]))) {
        gMockKeyQueue[gMockKeyTail++] = Key;
    }
}

/**
  Configure the next value returned by MockTuiReadHex().

  @param[in] Value          Hex value to return.
  @param[in] ShouldSucceed  TRUE if MockTuiReadHex() should succeed.
**/
void
MockTuiSetNextReadHex(
    NX_U64  Value,
    NX_BOOL ShouldSucceed
    )
{
    gMockNextHexValue = Value;
    gMockReadHexShouldSucceed = ShouldSucceed ? NX_TRUE : NX_FALSE;
}

/**
  Retrieve the last MaxDigits value passed to MockTuiReadHex().

  @return Last requested hex input digit limit.
**/
NX_UINTN
MockTuiGetLastReadHexMaxDigits(void)
{
    return gMockLastReadHexMaxDigits;
}

/**
  Reset mock TUI state.
**/
void
MockTuiReset(void)
{
    gMockKeyHead = 0;
    gMockKeyTail = 0;
    gMockNextHexValue = 0;
    gMockReadHexShouldSucceed = NX_TRUE;
    gMockLastReadHexMaxDigits = 0;

    MockTuiClearScreen();
}