/**
 * TuiMock.c - Mock implementation of TUI Adapter for testing
 */

#include <Uefi.h>
#include <stdarg.h>
#include "../../Adapter/AdapterInterface.h"

// Mock state
STATIC PM_UI_CHAR gMockScreenBuffer[80 * 24];
STATIC PM_UINTN gMockCursorColumn = 0;
STATIC PM_UINTN gMockCursorRow = 0;
STATIC PM_UINTN gMockLastAttribute = 0;
STATIC PM_UINTN gMockScreenWidth = 80;
STATIC PM_UINTN gMockScreenHeight = 24;
STATIC PM_UI_KEY gMockKeyQueue[32];
STATIC PM_UINTN gMockKeyHead = 0;
STATIC PM_UINTN gMockKeyTail = 0;
STATIC PM_U64 gMockNextHexValue = 0;
STATIC PM_BOOL gMockReadHexShouldSucceed = PM_TRUE;
STATIC PM_UINTN gMockLastReadHexMaxDigits = 0;

// Print capture buffer
STATIC PM_UI_CHAR gPrintCapture[8192];
STATIC PM_UINTN gPrintCaptureLen = 0;

STATIC VOID MockTuiClearScreen(VOID);
STATIC VOID MockTuiSetAttribute(PM_UINTN Attribute);
STATIC VOID MockTuiSetCursorPosition(PM_UINTN Column, PM_UINTN Row);
STATIC VOID MockTuiOutputString(CONST PM_UI_CHAR *String);
STATIC VOID MockTuiOutputStringAt(PM_UINTN Column, PM_UINTN Row, CONST PM_UI_CHAR *String);
STATIC VOID MockTuiDrawHeader(CONST PM_UI_CHAR *Title);
STATIC VOID MockTuiDrawFooter(CONST PM_UI_CHAR *Help);
STATIC PM_UI_KEY MockTuiReadKey(VOID);
STATIC PM_BOOL MockTuiReadHex(PM_U64 *Value, PM_UINTN MaxDigits);
STATIC PM_UI_STATUS MockTuiGetScreenDimensions(PM_UINTN *Columns, PM_UINTN *Rows);

STATIC TUI_ADAPTER_INTERFACE mTuiAdapterMock = {
    .ClearScreen = MockTuiClearScreen,
    .SetAttribute = MockTuiSetAttribute,
    .SetCursorPosition = MockTuiSetCursorPosition,
    .OutputString = MockTuiOutputString,
    .OutputStringAt = MockTuiOutputStringAt,
    .DrawHeader = MockTuiDrawHeader,
    .DrawFooter = MockTuiDrawFooter,
    .ReadKey = MockTuiReadKey,
    .ReadHex = MockTuiReadHex,
    .GetScreenDimensions = MockTuiGetScreenDimensions,
};

// ============================================================================
// Implementation
// ============================================================================

STATIC
VOID
MockTuiClearScreen(VOID)
{
    for (PM_UINTN i = 0; i < sizeof(gMockScreenBuffer) / sizeof(PM_UI_CHAR); i++) {
        gMockScreenBuffer[i] = L' ';
    }
    gMockCursorColumn = 0;
    gMockCursorRow = 0;
}

STATIC
VOID
MockTuiSetAttribute(UINTN Attribute)
{
    gMockLastAttribute = Attribute;
}

STATIC
VOID
MockTuiSetCursorPosition(UINTN Column, UINTN Row)
{
    gMockCursorColumn = Column;
    gMockCursorRow = Row;
}

STATIC
VOID
MockTuiOutputString(CONST PM_UI_CHAR *String)
{
    if (String == NULL) {
        return;
    }

    while (*String != L'\0' && gMockCursorColumn < gMockScreenWidth) {
        PM_UINTN Index = gMockCursorRow * gMockScreenWidth + gMockCursorColumn;
        gMockScreenBuffer[Index] = *String;
        gMockCursorColumn++;
        String++;
    }
}

VOID
MockTuiAppendString(IN CONST CHAR16 *String)
{
    if (String == NULL) {
        return;
    }

    while (*String != L'\0' && gMockCursorRow < gMockScreenHeight) {
        if (*String == L'\n') {
            gMockCursorColumn = 0;
            gMockCursorRow++;
            String++;
            continue;
        }

        if (gMockCursorColumn < gMockScreenWidth) {
            PM_UINTN Index = gMockCursorRow * gMockScreenWidth + gMockCursorColumn;
            gMockScreenBuffer[Index] = *String;
            gMockCursorColumn++;
        }
        String++;
    }
}

STATIC
VOID
MockTuiOutputStringAt(PM_UINTN Column, PM_UINTN Row, CONST PM_UI_CHAR *String)
{
    MockTuiSetCursorPosition(Column, Row);
    MockTuiOutputString(String);
}

STATIC
VOID
MockTuiDrawHeader(CONST PM_UI_CHAR *Title)
{
    MockTuiOutputStringAt(0, 0, Title);
}

STATIC
VOID
MockTuiDrawFooter(CONST PM_UI_CHAR *Help)
{
    MockTuiOutputStringAt(0, gMockScreenHeight - 1, Help);
}

STATIC
PM_UI_KEY
MockTuiReadKey(VOID)
{
    PM_UI_KEY Key = {0};

    if (gMockKeyHead < gMockKeyTail) {
        Key = gMockKeyQueue[gMockKeyHead++];
    }

    return Key;
}

STATIC
PM_BOOL
MockTuiReadHex(PM_U64 *Value, PM_UINTN MaxDigits)
{
    gMockLastReadHexMaxDigits = MaxDigits;
    if (Value != NULL) {
        *Value = gMockNextHexValue;
    }

    return gMockReadHexShouldSucceed;
}

STATIC
PM_UI_STATUS
MockTuiGetScreenDimensions(PM_UINTN *Columns, PM_UINTN *Rows)
{
    if (Columns != NULL) {
        *Columns = gMockScreenWidth;
    }
    if (Rows != NULL) {
        *Rows = gMockScreenHeight;
    }
    return PM_UI_SUCCESS;
}

void MockPrintCapture(CONST CHAR16 *Format, ...)
{
    va_list Args;
    PM_UINTN Available;

    if (Format == NULL) {
        return;
    }

    if (gPrintCaptureLen >= sizeof(gPrintCapture) / sizeof(PM_UI_CHAR)) {
        return;
    }

    Available = (sizeof(gPrintCapture) / sizeof(PM_UI_CHAR)) - gPrintCaptureLen;
    va_start(Args, Format);
    gPrintCaptureLen += vswprintf(
        &gPrintCapture[gPrintCaptureLen],
        Available,
        Format,
        Args
        );
    va_end(Args);
}

void TuiClearScreen(VOID)
{
}

VOID
TuiWaitForKeyPress(VOID)
{
}

/**
 * Get mock TUI adapter
 */
TUI_ADAPTER_INTERFACE*
GetMockTuiAdapter(VOID)
{
    return &mTuiAdapterMock;
}

/**
 * Get mock screen buffer content for verification
 */
CONST CHAR16*
MockTuiGetScreenBuffer(VOID)
{
    return gMockScreenBuffer;
}

/**
 * Get current mock cursor position
 */
VOID
MockTuiGetCursorPosition(OUT UINTN *Column, OUT UINTN *Row)
{
    if (Column != NULL) *Column = gMockCursorColumn;
    if (Row != NULL) *Row = gMockCursorRow;
}

VOID
MockTuiPushKey(PM_UI_KEY Key)
{
    if (gMockKeyTail < (sizeof(gMockKeyQueue) / sizeof(gMockKeyQueue[0]))) {
        gMockKeyQueue[gMockKeyTail++] = Key;
    }
}

VOID
MockTuiSetNextReadHex(UINT64 Value, BOOLEAN ShouldSucceed)
{
    gMockNextHexValue = Value;
    gMockReadHexShouldSucceed = ShouldSucceed ? PM_TRUE : PM_FALSE;
}

UINTN
MockTuiGetLastReadHexMaxDigits(VOID)
{
    return gMockLastReadHexMaxDigits;
}

VOID
MockTuiReset(VOID)
{
    gMockKeyHead = 0;
    gMockKeyTail = 0;
    gMockNextHexValue = 0;
    gMockReadHexShouldSucceed = PM_TRUE;
    gMockLastReadHexMaxDigits = 0;
    MockTuiClearScreen();
}

VOID
TuiMockCaptureReset(VOID)
{
    MockTuiReset();
    gPrintCaptureLen = 0;
    for (PM_UINTN i = 0; i < sizeof(gPrintCapture) / sizeof(PM_UI_CHAR); i++) {
        gPrintCapture[i] = L'\0';
    }
}

BOOLEAN
TuiMockCaptureContains(IN CONST CHAR16 *String)
{
    CONST wchar_t *Screen = (CONST wchar_t *)gPrintCapture;
    UINTN BufferSize = gPrintCaptureLen;

    if (String == NULL || String[0] == L'\0') {
        return FALSE;
    }

    for (UINTN Start = 0; Start + 1 <= BufferSize; Start++) {
        UINTN Index = 0;

        while (String[Index] != L'\0' && Start + Index < BufferSize &&
               Screen[Start + Index] == String[Index]) {
            Index++;
        }

        if (String[Index] == L'\0') {
            return TRUE;
        }
    }

    return FALSE;
}
