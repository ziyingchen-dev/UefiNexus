/**
 * TuiMock.c - Mock implementation of TUI Adapter for testing
 */

#include "../../Adapter/AdapterInterface.h"

// Mock state
static PM_UI_CHAR gMockScreenBuffer[80 * 24];
static PM_UINTN gMockCursorColumn = 0;
static PM_UINTN gMockCursorRow = 0;
static PM_UINTN gMockLastAttribute = 0;
static PM_UINTN gMockScreenWidth = 80;
static PM_UINTN gMockScreenHeight = 24;
static PM_UI_KEY gMockKeyQueue[32];
static PM_UINTN gMockKeyHead = 0;
static PM_UINTN gMockKeyTail = 0;
static PM_U64 gMockNextHexValue = 0;
static PM_BOOL gMockReadHexShouldSucceed = PM_TRUE;
static PM_UINTN gMockLastReadHexMaxDigits = 0;

static void MockTuiClearScreen(void);
static void MockTuiSetAttribute(PM_UINTN Attribute);
static void MockTuiSetCursorPosition(PM_UINTN Column, PM_UINTN Row);
static void MockTuiOutputString(const PM_UI_CHAR *String);
static void MockTuiOutputStringAt(PM_UINTN Column, PM_UINTN Row, const PM_UI_CHAR *String);
static void MockTuiDrawHeader(const PM_UI_CHAR *Title);
static void MockTuiDrawFooter(const PM_UI_CHAR *Help);
static PM_UI_KEY MockTuiReadKey(void);
static PM_BOOL MockTuiReadHex(PM_U64 *Value, PM_UINTN MaxDigits);
static PM_UI_STATUS MockTuiGetScreenDimensions(PM_UINTN *Columns, PM_UINTN *Rows);

static TUI_ADAPTER_INTERFACE mTuiAdapterMock = {
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

static
void
MockTuiClearScreen(void)
{
    for (PM_UINTN i = 0; i < sizeof(gMockScreenBuffer) / sizeof(PM_UI_CHAR); i++) {
        gMockScreenBuffer[i] = L' ';
    }
    gMockCursorColumn = 0;
    gMockCursorRow = 0;
}

static
void
MockTuiSetAttribute(PM_UINTN Attribute)
{
    gMockLastAttribute = Attribute;
}

static
void
MockTuiSetCursorPosition(PM_UINTN Column, PM_UINTN Row)
{
    gMockCursorColumn = Column;
    gMockCursorRow = Row;
}

static
void
MockTuiOutputString(const PM_UI_CHAR *String)
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

static
void
MockTuiOutputStringAt(PM_UINTN Column, PM_UINTN Row, const PM_UI_CHAR *String)
{
    MockTuiSetCursorPosition(Column, Row);
    MockTuiOutputString(String);
}

static
void
MockTuiDrawHeader(const PM_UI_CHAR *Title)
{
    MockTuiOutputStringAt(0, 0, Title);
}

static
void
MockTuiDrawFooter(const PM_UI_CHAR *Help)
{
    MockTuiOutputStringAt(0, gMockScreenHeight - 1, Help);
}

static
PM_UI_KEY
MockTuiReadKey(void)
{
    PM_UI_KEY Key = {0};

    if (gMockKeyHead < gMockKeyTail) {
        Key = gMockKeyQueue[gMockKeyHead++];
    }

    return Key;
}

static
PM_BOOL
MockTuiReadHex(PM_U64 *Value, PM_UINTN MaxDigits)
{
    gMockLastReadHexMaxDigits = MaxDigits;
    if (Value != NULL) {
        *Value = gMockNextHexValue;
    }

    return gMockReadHexShouldSucceed;
}

static
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

/**
 * Get mock TUI adapter
 */
TUI_ADAPTER_INTERFACE*
GetMockTuiAdapter(void)
{
    return &mTuiAdapterMock;
}

/**
 * Get mock screen buffer content for verification
 */
const PM_UI_CHAR*
MockTuiGetScreenBuffer(void)
{
    return gMockScreenBuffer;
}

/**
 * Get current mock cursor position
 */
void
MockTuiGetCursorPosition(PM_UINTN *Column, PM_UINTN *Row)
{
    if (Column != NULL) *Column = gMockCursorColumn;
    if (Row != NULL) *Row = gMockCursorRow;
}

void
MockTuiPushKey(PM_UI_KEY Key)
{
    if (gMockKeyTail < (sizeof(gMockKeyQueue) / sizeof(gMockKeyQueue[0]))) {
        gMockKeyQueue[gMockKeyTail++] = Key;
    }
}

void
MockTuiSetNextReadHex(PM_U64 Value, PM_BOOL ShouldSucceed)
{
    gMockNextHexValue = Value;
    gMockReadHexShouldSucceed = ShouldSucceed ? PM_TRUE : PM_FALSE;
}

PM_UINTN
MockTuiGetLastReadHexMaxDigits(void)
{
    return gMockLastReadHexMaxDigits;
}

void
MockTuiReset(void)
{
    gMockKeyHead = 0;
    gMockKeyTail = 0;
    gMockNextHexValue = 0;
    gMockReadHexShouldSucceed = PM_TRUE;
    gMockLastReadHexMaxDigits = 0;
    MockTuiClearScreen();
}
