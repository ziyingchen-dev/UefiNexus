/**
 * TuiAdapter.c - Wraps TuiLib to implement TUI_ADAPTER_INTERFACE
 * This adapter abstracts TUI operations for testability
 */
#include "../AdapterInterface.h"
#include "UefiBridge.h"

// Forward declarations
static void TuiAdapterClearScreen(void);
static void TuiAdapterSetAttribute(PM_UINTN Attribute);
static void TuiAdapterSetCursorPosition(PM_UINTN Column, PM_UINTN Row);
static void TuiAdapterOutputString(const PM_UI_CHAR *String);
static void TuiAdapterOutputStringAt(PM_UINTN Column, PM_UINTN Row, const PM_UI_CHAR *String);
static void TuiAdapterDrawHeader(const PM_UI_CHAR *Title);
static void TuiAdapterDrawFooter(const PM_UI_CHAR *Help);
static PM_UI_KEY TuiAdapterReadKey(void);
static PM_BOOL TuiAdapterReadHex(PM_U64 *Value, PM_UINTN MaxDigits);
static PM_UI_STATUS TuiAdapterGetScreenDimensions(PM_UINTN *Columns, PM_UINTN *Rows);

// Static interface instance
static TUI_ADAPTER_INTERFACE mTuiAdapter = {
    .ClearScreen = TuiAdapterClearScreen,
    .SetAttribute = TuiAdapterSetAttribute,
    .SetCursorPosition = TuiAdapterSetCursorPosition,
    .OutputString = TuiAdapterOutputString,
    .OutputStringAt = TuiAdapterOutputStringAt,
    .DrawHeader = TuiAdapterDrawHeader,
    .DrawFooter = TuiAdapterDrawFooter,
    .ReadKey = TuiAdapterReadKey,
    .ReadHex = TuiAdapterReadHex,
    .GetScreenDimensions = TuiAdapterGetScreenDimensions,
};

// ============================================================================
// Implementation
// ============================================================================

static
void
TuiAdapterClearScreen(void)
{
    UefiBridgeTuiClearScreen();
}

static
void
TuiAdapterSetAttribute(PM_UINTN Attribute)
{
    UefiBridgeTuiSetAttribute(Attribute);
}

static
void
TuiAdapterSetCursorPosition(PM_UINTN Column, PM_UINTN Row)
{
    UefiBridgeTuiSetCursorPosition(Column, Row);
}

static
void
TuiAdapterOutputString(const PM_UI_CHAR *String)
{
    UefiBridgeTuiOutputString(String);
}

static
void
TuiAdapterOutputStringAt(PM_UINTN Column, PM_UINTN Row, const PM_UI_CHAR *String)
{
    UefiBridgeTuiOutputStringAt(Column, Row, String);
}

static
void
TuiAdapterDrawHeader(const PM_UI_CHAR *Title)
{
    UefiBridgeTuiDrawHeader(Title);
}

static
void
TuiAdapterDrawFooter(const PM_UI_CHAR *Help)
{
    UefiBridgeTuiDrawFooter(Help);
}

static
PM_UI_KEY
TuiAdapterReadKey(void)
{
    return UefiBridgeTuiReadKey();
}

static
PM_BOOL
TuiAdapterReadHex(PM_U64 *Value, PM_UINTN MaxDigits)
{
    return UefiBridgeTuiReadHex(Value, MaxDigits);
}

static
PM_UI_STATUS
TuiAdapterGetScreenDimensions (
  PM_UINTN  *Columns,
  PM_UINTN  *Rows
  )
{
  return UefiBridgeTuiGetScreenDimensions(Columns, Rows);
}

/**
 * Get the TUI adapter interface instance
 */
TUI_ADAPTER_INTERFACE*
GetTuiAdapter(void)
{
    return &mTuiAdapter;
}
